<?php
/**
 * @file
 * Cache class wrapper for non-transactional backend.
 *
 * The class works in two modes: safe or non-safe.
 *
 * Both modes buffers cache operations during transactions, in order to keep
 * cache in sync with database contents.
 *
 * In safe-mode however, cache_set() is silenty ignored/discarded during
 * transactions.
 *
 * Safe mode is needed for fully eliminating the race conditions that cause
 * the cache to come out of sync with the DB, if the REPEATABLE-READ isolation
 * level is used.
 *
 * If using the READ-COMMITTED isolation level, safe mode should not be enabled.
 *
 * @code
 *  variable_set('consistent_cache_default_safe', FALSE);
 * @endcode
 */

/**
 * Defines a transactional aware cache implementation.
 *
 * Buffer/discard cache operations during transactions.
 */
class ConsistentCache implements DrupalTransactionalCacheInterface {
  protected $backend = NULL;
  protected $bin;
  protected $buffer = array();
  protected $safe = FALSE;
  static protected $backends = array();

  /**
   * Constructor.
   *
   * Initialize bin.
   */
  public function __construct($bin) {
    $this->bin = $bin;

    // Let our parent do its work.
    $class = variable_get('consistent_cache_class_' . $bin);
    if (!isset($class)) {
      $class = variable_get('consistent_cache_default_class', 'DrupalDatabaseCache');
    }

    // Check for safe mode.
    $this->safe = variable_get('consistent_cache_safe_' . $bin);
    if (!isset($this->safe)) {
      $this->safe = variable_get('consistent_cache_default_safe', TRUE);
    }

    if (!isset(self::$backends[$bin])) {
      self::$backends[$bin] = new $class($bin);
    }
    $this->backend = &self::$backends[$bin];
  }

  /**
   * Destructor.
   *
   * Flush remaining buffers, if any.
   */
  public function __destruct() {
    $this->commit(0);
    unset(self::$backends[$this->bin]);
  }

  /**
   * Implements DrupalCacheInterface::get().
   */
  public function get($cid) {
    $cids = array($cid);
    $cache = $this->getMultiple($cids);
    return reset($cache);
  }

  /**
   * Find an entry in the buffer that matches the $cid.
   *
   * ... with or without wildcard.
   *
   * @param string $cid
   *   The cache id to look up.
   *
   * @return array
   *   Cache operation if found, NULL otherwise.
   */
  protected function matchKey($cid) {
    if (!empty($this->buffer)) {
      $operation = end($this->buffer);
      do {
        if (strpos($cid, $operation[2]) === 0) {
          return $operation;
        }
      } while ($operation = prev($this->buffer));
    }
  }

  /**
   * Implements DrupalCacheInterface::getMultiple().
   *
   * Get multiple cache entries taking buffer into account.
   */
  public function getMultiple(&$cids) {
    $ocids = $cids;
    $return = $this->backend->getMultiple($cids);

    // If buffer is not empty, then check for changes in $cids requested.
    if (!empty($this->buffer)) {
      $cids = array_flip($cids);
      foreach ($ocids as $cid) {
        if ($operation = $this->matchKey($cid)) {
          switch ($operation[1]) {
            case 'set':
              $return[$cid] = (object) array(
                'cid' => $cid,
                'data' => $operation[3],
                'created' => time(),
                'headers' => NULL,
                'flushes' => 0,
                'expire' => $operation[4],
              );
              unset($cids[$cid]);
              break;

            case 'clear':
              unset($return[$cid]);
              $cids[$cid] = TRUE;
              break;

          }
        }
      }
      $cids = array_keys($cids);
    }
    return $return;
  }

  /**
   * Implements DrupalCacheInterface::set().
   *
   * Buffer cache operation if inside transaction.
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT) {
    $depth = drupal_bootstrap(NULL, FALSE) >= DRUPAL_BOOTSTRAP_DATABASE ? Database::getConnection()->transactionDepth() : 0;
    if ($depth == 0) {
      // Don't buffer untransactionalized cache operations.
      return $this->backend->set($cid, $data, $expire);
    }
    elseif (!$this->safe) {
      // Only buffer during transactions when safe mode is off.
      // Otherwise, discard cache set.
      $this->buffer[] = array(
        $depth,
        'set',
        $cid,
        $this->deepClone($data),
        $expire,
      );
    }
  }

  /**
   * Implements DrupalCacheInterface::clear().
   *
   * Buffer cache operation if inside transaction.
   */
  public function clear($cid = NULL, $wildcard = FALSE) {
    // Explicit memcache support :-(
    if ($this->backend instanceOf MemCacheDrupal) {
      $backtrace = debug_backtrace();
      if ($cid == MEMCACHE_CONTENT_CLEAR || (isset($backtrace[2]) && $backtrace[2]['function'] == 'cache_clear_all' && empty($backtrace[2]['args']))) {
        $cid = MEMCACHE_CONTENT_CLEAR;
      }
    }

    $depth = drupal_bootstrap(NULL, FALSE) >= DRUPAL_BOOTSTRAP_DATABASE ? Database::getConnection()->transactionDepth() : 0;
    if ($depth == 0) {
      // Don't buffer untransactionalized cache operations.
      return $this->backend->clear($cid, $wildcard);
    }
    else {
      $cids = is_array($cid) ? $cid : array($cid);
      foreach ($cids as $cid) {
        $this->buffer[] = array($depth, 'clear', $cid, $wildcard);
      }
    }
  }

  /**
   * Implements DrupalCacheInterface::isEmpty().
   */
  public function isEmpty() {
    // If anything is present in the buffer, then we cannot determine if the
    // bin is empty.
    if (!empty($this->buffer)) {
      return FALSE;
    }

    // If inside a transaction in safe mode, then nothing is certain.
    if ($this->safe) {
      $depth = drupal_bootstrap(NULL, FALSE) >= DRUPAL_BOOTSTRAP_DATABASE ? Database::getConnection()->transactionDepth() : 0;
      if ($depth) {
        return FALSE;
      }
    }

    // Nothing in buffer nor safe+transaction; ask the backend.
    return $this->backend->isEmpty();
  }

  /**
   * Implements DrupalTransactionalCacheInterface::rollback().
   *
   * Clear cache operation buffer.
   *
   * Clear all affected cids from cache.
   * This is called only during rollback scenarios.
   */
  public function rollback($depth) {
    foreach ($this->buffer as $idx => $operation) {
      if ($operation[0] > $depth) {
        unset($this->buffer[$idx]);
      }
    }
  }

  /**
   * Implements DrupalTransactionalCacheInterface::rollback().
   *
   * Flush cache operation buffer.
   */
  public function commit($depth) {
    if ($depth > 0) {
      // Nested commit. Don't flush yet, but move all operations to
      // parent depth (pseudo-commit).
      foreach ($this->buffer as $idx => &$operation) {
        if ($operation[0] > $depth) {
          $operation[0] = $depth;
        }
      }
      return;
    }

    while (!empty($this->buffer)) {
      $operation = array_shift($this->buffer);
      $depth = array_shift($operation);
      $method = array_shift($operation);
      call_user_func_array(array($this->backend, $method), $operation);
    }
  }

  /**
   * Deep clone data structure.
   *
   * @param mixed $data
   *   The data structure to clone.
   *
   * @return mixed
   *   Cloned data structure.
   */
  private function deepClone($data) {
    if (is_array($data)) {
      $result = array();
      foreach ($data as $key => $value) {
        $result[$key] = $this->deepClone($value);
      }
      return $result;
    }
    elseif (is_object($data)) {
      return clone $data;
    }
    else {
      return $data;
    }
  }
}
