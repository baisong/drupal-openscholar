<?php

/**
 * @file
 * Hooks provided by the oEmbed module.
 */

/**
 * Alters an oEmbed request parameters and provider.
 *
 * @param array $parameters
 *   oEmbed request parameters.
 * @param object $provider
 *   oEmbed provider info.
 * @param string $url
 *   The original URL or embed code to parse.
 */
function hook_oembed_request_alter(&$parameters, &$provider, $url) {
  if ($provider['name'] == 'youtube') {
    $parameters['iframe'] = '1';
  }
}

/**
 * Alters an oEmbed response.
 *
 * @param array $response
 *   oEmbed response data.
 */
function hook_oembed_response_alter(&$response) {
}

/**
 * Modify the provider's set of supported oEmbed response formats.
 *
 * @param array $formats
 *   Format handlers keyed by format name.
 */
function hook_oembedprovider_formats_alter(&$formats) {
  $formats['jsonp'] = array(
    'mime' => 'text/javascript',
    'callback' => '_oembedprovider_formats_jsonp',
  );
}

/**
 * Allows adding fragments to an oembed url. These fragments are ignored by the oembed spec,
 * but can be used to differentiate between different instances of the same URL.
 *
 * Ex.
 *    https://www.youtube.com/watch?v=C8h4eZ4DL9Q#results/of/hook
 *
 * @param string $uri
 *    URI being added to the system
 *
 * @return array $fragments
 *    Array of strings to be appended to the uri as URL fragments
 */
function hook_oembed_uri_fragment($uri) {
  return array(
    'identifyingString'
  );
}