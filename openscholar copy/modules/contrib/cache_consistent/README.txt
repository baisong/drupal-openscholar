Two methods of ensuring cache consistency are bundled with this module.

== Configuration ==

Apply the core patch included with the module. See https://www.drupal.org/node/1679344.

A transactional aware cache wrapper, that buffers cache operations until transaction has been committed. This method works best with the database isolation level is set to READ-COMMITTED. This method CAN be used with REPEATABLE-READ, but will in this case only mitigate the problem, not eliminate it.

=== Method #1 ===

$conf['cache_backends'] = array(
  'sites/all/modules/memcache/memcache.inc',
  'sites/all/modules/cache_consistent/cache_consistent.inc',
);
$conf['cache_default_class'] = 'ConsistentCache';
$conf['consistent_cache_default_class'] = 'MemCacheDrupal';
$conf['consistent_cache_default_safe'] = FALSE;



=== Method #2 (ignore cache set during transactions) ====

A transactional aware cache wrapper, that buffers cache operations until transaction has been committed. In this method, cache set is ignored when inside a transaction. This method is the most robust for setups using isolation level REPEATABLE-READ.

$conf['cache_backends'] = array(
  'sites/all/modules/memcache/memcache.inc',
  'sites/all/modules/cache_consistent/cache_consistent.inc',
);
$conf['cache_default_class'] = 'ConsistentCache';
$conf['consistent_cache_default_class'] = 'MemCacheDrupal';
$conf['consistent_cache_default_safe'] = TRUE;


=== Alternate cache class ===
An alternate version of the ConsistentCache class is also available.

It's called ConsistentCacheLookup and resides in cache_consistent.lookup.inc instead of cache_consistent.inc

The ConsistentCacheLookup class has a more intelligent buffering mechanism, which can result in fewer actual cache operations.

=== Method #1 (alternate) ===

$conf['cache_backends'] = array(
  'sites/all/modules/memcache/memcache.inc',
  'sites/all/modules/cache_consistent/cache_consistent.lookup.inc',
);
$conf['cache_default_class'] = 'ConsistentCacheLookup';
$conf['consistent_cache_default_class'] = 'MemCacheDrupal';
$conf['consistent_cache_default_safe'] = FALSE;

=== Method #2 (alternate) ===

$conf['cache_backends'] = array(
  'sites/all/modules/memcache/memcache.inc',
  'sites/all/modules/cache_consistent/cache_consistent.lookup.inc',
);
$conf['cache_default_class'] = 'ConsistentCacheLookup';
$conf['consistent_cache_default_class'] = 'MemCacheDrupal';
$conf['consistent_cache_default_safe'] = TRUE;

