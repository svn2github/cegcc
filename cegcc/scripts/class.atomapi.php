<?php

/****************************************************************************\
                         PHP Atom API Package
                             version 1.0

         Includes AtomAPI, AtomFeed, AtomEntry and AtomRequest

                     Written by Beau Lebens, 2005
                       beau@dentedreality.com.au
              http://www.dentedreality.com.au/phpatomapi/


   More Atom Info;
     AtomEnabled: http://www.atomenabled.org/
     Atom Wiki: http://www.intertwingly.net/wiki/pie/FrontPage
     API Spec: http://www.atomenabled.org/developers/api/atom-api-spec.php
    
   Blog-Specific API Details
     Blogger: http://code.blogspot.com/archives/atom-docs.html
     TypePad: http://sixapart.com/developers/atom/typepad/

   TODO
     - Proper date handling (GMT, local etc)
     - AtomEntry::from_xml() needs work on author info handling - sample?
     - Complete test suite/application?
\****************************************************************************/

// Error Numbers
define('ATOMAPI_ENDPOINT_INVALID',			1);
define('ATOMAPI_AUTHTYPE_INVALID',			2);
define('ATOMAPI_AUTHENTICATION_FAILED',	3);
define('ATOMAPI_NO_FEEDS',						4);
define('ATOMAPI_NO_XML',						5);
define('ATOMAPI_FEED_INIT_FAILED',			6);
define('ATOMAPI_METHOD_INVALID',				7);
define('ATOMAPI_PAYLOAD_INVALID',			8);
define('ATOMAPI_REQUEST_INVALID',			9);
define('ATOMAPI_CURL_ERROR',					10);

// Error Strings
$ATOMAPI_ERROR_STRINGS = array(1=>'The specified AtomAPI endpoint (or request URL) is invalid. Please enter a complete URL, starting with \'http\'.',
		2=>'The PHPAtomAPI Package currently only supports \'Basic\' and \'WSSE\' authentication models. Please specify one of them.',
		3=>'Authentication against the AtomAPI endpoint failed. Please check your username/password.',
		4=>'No feeds were located at the specified AtomAPI endpoint.',
		5=>'No feeds were initiated because there was no XML available to parse.',
		6=>'No feeds were initiated because the FeedURI did not respond as expected.',
		7=>'An invalid method was supplied to an AtomRequest object. Methods available are GET, PUT, POST and DELETE',
		8=>'The payload supplied to an AtomRequest object appears to be something other than a string. Please only supply a string as payload.',
		9=>'An AtomRequest object was initiated incorrectly, and is missing either a URI or a Method.',
		10=>'cURL failed to perform the required request within an AtomRequest.');

class AtomAPI {
	var $endpoint;
	var $authtype;
	var $auth;
	var $feeds;
	var $feed_objs;
	var $err_no;
	
	/**
	* @return AtomAPI/FALSE
	* @param String $endpoint
	* @param String $username
	* @param String $password
	* @param String $authtype
	* @desc Creates an AtomAPI Object, verifying authentication details by attempting to load a list of feeds. $authtype should be 'Basic' or 'WSSE'
	*/
	function AtomAPI($endpoint, $username, $password, $authtype) {
		// Abort if the endpoint is missing
		if ($endpoint === false || !strlen($endpoint)) {
			$this->err_no = ATOMAPI_ENDPOINT_INVALID;
			return false;
		}
		
		// Set the endpoint and authtype
		$this->set_authtype($authtype, $username, $password);
		if (!$this->get_auth()) {
			return false;
		}
		$this->set_endpoint($endpoint);
		if (!$this->get_endpoint()) {
			return false;
		}
		
		// Verify authentication (and load feeds)
		return $this->verify_auth();
	}
	
	/**
	* @return void
	* @param String $uri
	* @desc Set the endpoint for accessing this AtomAPI. Must be a fully-qualified URI starting with 'http'
	*/
	function set_endpoint($uri) {
		if (substr($uri, 0, 4) == 'http') {
			$this->endpoint = $uri;
		}
		else {
			$this->err_no = ATOMAPI_ENDPOINT_INVALID;
		}
	}
	
	/**
	* @return String/FALSE
	* @desc Return the endpoint set for this AtomAPI or FALSE if unset
	*/
	function get_endpoint() {
		if (isset($this->endpoint)) {
			return $this->endpoint;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $type
	* @param String $user
	* @param String $pass
	* @desc Creates the authentication object for this Atom API, based on the type specified. Currently supports WSSE and Basic as $type
	*/
	function set_authtype($type, $user, $pass) {
		$this->authtype = $type;
		
		if ($type == 'Basic') {
			require_once(dirname(__FILE__) . '/class.basicauth.php');
			$this->auth = new BasicAuth($user, $pass);
		}
		else if ($type == 'WSSE') {
			require_once(dirname(__FILE__) . '/class.wsse.php');
			$this->auth = new WSSE($user, $pass);
		}
		else {
			$this->err_no = ATOMAPI_AUTHTYPE_INVALID;
		}
	}
	
	/**
	* @return String
	* @desc Returns the current authentication model (Basic or WSSE)
	*/
	function get_authtype() {
		return $this->authtype;
	}
	
	/**
	* @return Boolean
	* @desc Verifies authentication details by attempting to access the endpoint. If successful, initiates the list of available feeds and returns true, otherwise returns false.
	*/
	function verify_auth() {
		// Test authentication by accessing the endpoint with the auth object
		$ar = new AtomRequest('GET', $this->get_endpoint(), $this->auth, '');
		$response = $ar->exec();
		
		// Looks ok, return the Object
		if ($response == 200) {
			$this->get_feeds($ar->get_response());
			return true;
		}
		// Not a 200 Ok response - authetication failed, or endpoint incorrect
		else {
			$this->err_no = ATOMAPI_AUTHENTICATION_FAILED;
			return false;
		}
	}
	
	/**
	* @return AuthenticationObject
	* @desc Returns an authentication object (either WSSE or Basic)
	*/
	function get_auth() {
		return $this->auth;
	}
	
	/**
	* @return Array/String
	* @desc Returns an array containing the details of the feeds listed at the endpoint. On failure, returns the HTTPCode as a string instead
	*/
	function get_feeds($xml = false) {
		// Feeds already fetched? Then just return the array again
		if (is_array($this->feeds)) {
			return $this->feeds;
		}
		// First time the feeds are requested, so get the endpoint and build out the array
		else {
			if ($xml !== false) {
				$feeds_raw = $xml;
			}
			else {
				// Create an AtomRequest pointing to the AtomAPI endpoint (where we get a list of available feeds)
				$ar = new AtomRequest('GET', $this->endpoint, $this->auth, '');
				$code = $ar->exec();
				if ($code) {
					$feeds_raw = $ar->get_response();
				}
				else {
					$feeds_raw = false;
				}
			}
			
			// Request was successful
			if ($feeds_raw) {
				// Get the body of the response and parse out all <link>s
				preg_match_all('/(<link[^>]+rel="service.(feed|post)"[^>]*\/>)/Ui', $feeds_raw, $matches);
				
				if (strlen($matches[0][0])) {
					$feeds = array();
					// Build an array containing the details of all located <links>
					// <link>s are currently "paired" according to their URLs, so that the following is built;
					// $feeds = array('title'=>$title, 'service.feed'=>$FeedURI, 'service.edit'=>$EditURI);
					foreach ($matches[1] as $l=>$link) {
						preg_match('/href="([^"]+)"/i', $link, $href);
						preg_match('/title="([^"]+)"/i', $link, $title);
						foreach ($feeds as $f=>$feed) {
							if ($feed['service.post'] == $href[1] || $feed['service.feed'] == $href[1]) {
								$feeds[$f]['service.' . $matches[2][$l]] = $href[1];
								$feeds[$f]['title'] = $title[1];
								continue 2;
							}
						}
						$feeds[] = array('service.' . $matches[2][$l]=>$href[1], 'title'=>$title[1]);
					}
					
					$this->feeds = $feeds;
					return $feeds;
				}
				// No <link>s found - abort and return FALSE
				else {
					$this->err_no = ATOMAPI_NO_FEEDS;
					return false;
				}
			}
			// Request failed
			else {
				$this->err_no = ATOMAPI_NO_XML;
				return $code;
			}
		}
	}
	
	/**
	* @return AtomFeed
	* @param Int $id
	* @desc Creates (if required) and returns an AtomFeed object, based on an $id for the feeds array (pulled from endpoint)
	*/
	function get_feed($id) {
		// Build the feeds array if it's not already initiated
		if (!sizeof($this->feeds)) {
			$this->get_feeds();
		}
		
		// And create this AtomFeed if it's not there
		if (!is_object($this->feed_objs[$id])) {
			$this->feed_objs[$id] = new AtomFeed($this->feeds[$id]['service.feed'], $this->get_auth());
		}
		
		// Return false if there's still no object
		if (!is_object($this->feed_objs[$id])) {
			return false;
		}
		// Return the requested AtomFeed
		else {
			return $this->feed_objs[$id];
		}
	}
	
	/**
	* @return Array/String
	* @param Int $id
	* @desc Creates an AtomFeed object for the details specified in $id, and gets the entries available. Stores the object internally for later
	*/
	function get_entries($id) {
		// If the AtomFeed where entries are requested from doesn't exist, create it first
		if (!is_object($this->feed_objs[$id])) {
			$this->feed_objs[$id] = new AtomFeed($this->feeds[$id]['service.feed'], $this->auth);
		}
		
		// Return false if there's still no object
		if (!is_object($this->feed_objs[$id])) {
			return false;
		}
		// Request all entries available from this AtomFeed and return whatever comes back
		else {
			$entries = $this->feed_objs[$id]->get_entries();
			return $entries;
		}
	}
	
	/**
	* @return Int/FALSE
	* @desc Returns the last error registered in this class, or FALSE if none
	*/
	function error() {
		if (isset($this->err_no) && is_int($this->err_no)) {
			return $this->err_no;
		}
		else {
			return false;
		}
	}
}



class AtomFeed {
	var $endpoint;
	var $auth;
	var $title = array();
	var $author = array();
	var $version;
	var $links = array();
	var $tagline = array();
	var $id;
	var $generator = array();
	var $info = array();
	var $modified;
	var $entries = array();
	var $err_no;
	
	/**
	* @return AtomFeed
	* @param String $endpoint
	* @param AuthObj $auth
	* @param String $xml
	* @desc Creates an AtomFeed object, based on the details specified. If $xml is provided, then it is parsed to populate the object, otherwise the endpoint is accessed and it is populated from the response XML. If all values are false or omitted, then an empty AtomFeed is created, and awaits the use of set_*() functions to set it up manually.
	*/
	function AtomFeed($endpoint = false, $auth = false, $xml = false) {
		// If they are set, then store the endpoint and auth object
		if ($endpoint && $auth) {
			$this->set_endpoint($endpoint);
			$this->set_auth($auth);
		}
		
		// If the object was created with XML, populate it with parsed values
		if ($xml !== false) {
			$this->from_xml($xml);
		}
		// No XML? If there's an endpoint and auth object, then get some XML to populate with
		else if ($endpoint && $auth) {
			$res = $this->init();
			// >= 300 is not a good http status code to get back
			if ($res >= 300) {
				return false;
			}
		}
		else {
			$this->err_no = ATOMAPI_FEED_INIT_FAILED;
			return false;
		}
	}
	
	/**
	* @return void
	* @param AuthObj $auth
	* @desc Sets the authentication object to use for this AtomFeed
	*/
	function set_auth($auth) {
		if (is_object($auth)) {
			$this->auth = $auth;
		}
		else {
			$this->err_no = ATOMAPI_AUTHTYPE_INVALID;
		}
	}
	
	/**
	* @return AuthObj/FALSE
	* @desc Returns the currently set authentication object for this AtomFeed, or FALSE if not set
	*/
	function get_auth() {
		if (is_object($this->auth)) {
			return $this->auth;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $uri
	* @desc Set the endpoint for accessing this AtomFeed. Must be a fully-qualified URI starting with 'http'
	*/
	function set_endpoint($uri) {
		if (substr($uri, 0, 4) == 'http') {
			$this->endpoint = $uri;
		}
		else {
			$this->err_no = ATOMAPI_ENDPOINT_INVALID;
		}
	}
	
	/**
	* @return String/FALSE
	* @desc Returns the endpoint for this AtomFeed if specified, or FALSE if not
	*/
	function get_endpoint() {
		if (isset($this->endpoint) && strlen($this->endpoint)) {
			return $this->endpoint;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $v
	* @desc Set the version number for this AtomFeed (goes in the <feed> element)
	*/
	function set_version($v) {
		$this->version = $v;
	}
	
	/**
	* @return String
	* @desc Returns the version number specified for this AtomFeed in the <feed> element
	*/
	function get_version() {
		if (isset($this->version)) {
			return $this->version;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $i
	* @desc Sets the <id> element contents for the AtomFeed.
	*/
	function set_id($i) {
		$this->id = $i;
	}
	
	/**
	* @return String
	* @desc Returns the contents of the <id> element for this AtomFeed
	*/
	function get_id() {
		if (isset($this->id)) {
			return $this->id;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param Array/String $title
	* @desc Sets the title for this AtomFeed.
	*/
	function set_title($title) {
		if (is_array($title)) {
			foreach ($title as $key=>$val) {
				if (strlen($key) && strlen($val)) {
					$this->title[$key] = $val;
				}
			}
		}
		else if (is_string($title)) {
			$this->title['title'] = $title;
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @param String $elem
	* @desc Returns either a specific element of the title, or the entire array of data if none is specified.
	*/
	function get_title($elem = false) {
		if ($elem === false) {
			return $this->title;
		}
		else {
			if (in_array($elem, array_keys($this->title))) {
				return $this->title[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param Array/String $tagline
	* @desc Sets the details of the tagline for this AtomFeed
	*/
	function set_tagline($tagline) {
		if (is_array($tagline)) {
			foreach ($tagline as $key=>$val) {
				$this->tagline[$key] = $val;
			}
		}
		else if (is_string($tagline)) {
			$this->tagline['tagline'] = $tagline;
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @param String $elem
	* @desc Returns either a specific element of the tagline, or the entire array of data if none is specified.
	*/
	function get_tagline($elem = false) {
		if ($elem === false) {
			return $this->tagline;
		}
		else {
			if (in_array($elem, array_keys($this->tagline))) {
				return $this->tagline[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param Array $gen
	* @desc Sets the details of the generator for this AtomFeed.
	*/
	function set_generator($gen) {
		if (is_array($gen)) {
			foreach ($gen as $key=>$val) {
				$this->generator[$key] = $val;
			}
		}
	}
	
	/**
	* @return void
	* @param String $date
	* @desc Set the modified date for this feed
	*/
	function set_modified($date) {
		if (preg_match('/(\d{4})-(\d{2})-(\d{2}).(\d{2}):(\d{2}):(\d{2})-(\d{2}):(\d{2})/', $date, $parts) || preg_match('/(\d{4})-(\d{2})-(\d{2}).(\d{2}):(\d{2}):(\d{2})Z/', $date, $parts)) {
			$this->modified = $date;
		}
	}
	
	/**
	* @return String/FALSE
	* @desc Get the modified date for this feed, or FALSE if it's not set
	*/
	function get_modified() {
		if (isset($this->modified) && strlen($this->modified)) {
			return $this->modified;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param Array/String $info
	* @desc Set the info element details. If a string is passed in, then defaults to the info contents
	*/
	function set_info($info) {
		if (is_array($info)) {
			foreach ($info as $key=>$val) {
				$this->info[$key] = $val;
			}
		}
		else {
			$this->info['info'] = $info;
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @desc Returns either the requested individual attribute of the info element, the entire array, or FALSE if it's not set.
	*/
	function get_info($elem = false) {
		if ($elem === false) {
			return $this->info;
		}
		else {
			if (in_array($elem, array_keys($this->info))) {
				return $this->info[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @param String $elem
	* @desc Returns either a specific element of the generator, or the entire array of data if none is specified.
	*/
	function get_generator($elem = false) {
		if ($elem === false) {
			return $this->generator;
		}
		else {
			if (in_array($elem, array_keys($this->generator))) {
				return $this->generator[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param String $href
	* @param String $rel
	* @param String $title
	* @param String $type
	* @desc Adds a link to the links array with the details as per the associative array passed in. Requires href, rel, title and type
	*/
	function add_link($href, $rel, $title = '', $type) {
		if (strlen($href) && strlen($rel) && strlen($type)) {
			$this->links[] = array('href'=>$href, 'rel'=>$rel, 'title'=>$title, 'type'=>$type);
		}
	}
	
	/**
	* @return Array
	* @param String $key
	* @param String $val
	* @desc Filters the <link>s of this AtomFeed for ones where the $key matches the $val and returns an array of them, otherwise returns all links
	*/
	function get_links($key = false, $val = false) {
		// If key and val are specified, then filter the array for entries where the specified key == the value
		if ($key !== false && $val !== false) {
			$out = array();
			foreach ($this->links as $link) {
				if ($link[$key] == $val) {
					$out[] = $link;
				}
			}
		}
		else {
			$out = $this->links;
		}
		
		return $out;
	}
	
	/**
	* @return Array/FALSE
	* @desc Returns an array containing AtomEntries for each entry in this AtomFeed, or FALSE if none are set
	*/
	function get_entries() {
		if (sizeof($this->entries)) {
			return $this->entries;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param AtomEntry $entry
	* @desc Add an AtomEntry object to the array of entries that make up this AtomFeed
	*/
	function add_entry($entry) {
		if (is_object($entry)) {
			$this->entries[] = $entry;
		}
	}
	
	/**
	* @return TRUE/Int on error
	* @desc Initiates the AtomFeed object from a URI by loading the data, then parsing it as XML. Returns an HTTP response code on error, or TRUE if successful.
	*/
	function init() {
		$ar = new AtomRequest('GET', $this->get_endpoint(), $this->get_auth());
		$code = $ar->exec();
		
		// Successfully retrieved feed, now process it out
		if ($code == 200) {
			$this->from_xml($ar->get_response());
			return true;
		}
		// Not a clean result, return the error
		else {
			$this->err_no = ATOMAPI_FEED_INIT_FAILED;
			return $code;
		}
	}
	
	/**
	* @return Array/FALSE
	* @param String $str
	* @desc Parses an XML/HTML string and returns an associative array containing key/value pairs
	*/
	function extract_attribs($str) {
		$out = array();
		if (preg_match_all('/([^ =]+)="([^"]*)"/si', $str, $attribs)) {
			foreach ($attribs[1] as $c=>$key) {
				$out[$key] = $attribs[2][$c];
			}
		}
		return $out;
	}
	
	/**
	* @return void
	* @param String $xml
	* @desc Populates the variables of the AtomFeed, based on a complete XML representation of it.
	*/
	function from_xml($xml) {
		$orig_xml = $xml;
		
		// Strip down the XML to just the part we want to work with
		if (preg_match('/(<feed.*)<entry/sUi', $xml, $feed_xml)) {
			$xml = $feed_xml[1];
		}
		
		// ATOM FEED VERSION
		if (preg_match('/<feed[^>]+version="([^"]*)"/is', $xml, $ver)) {
			$this->set_version($ver[1]);
		}
		
		// TITLE
		if (preg_match_all('/<title([^>]*)>(.*)<\/title>/sUi', $xml, $title)) {
			$title_attribs = $this->extract_attribs($title[1][0]);
			$title = array('title'=>$title[2][0]);
			$title = array_merge_recursive($title, $title_attribs);
			$this->set_title($title);
		}
		
		// TAGLINE
		if (preg_match_all('/<tagline([^>]*)>(.*)<\/tagline>/sUi', $xml, $tagline)) {
			$tagline_attribs = $this->extract_attribs($tagline[1][0]);
			$tagline = array('tagline'=>$tagline[2][0]);
			$tagline = array_merge_recursive($tagline, $tagline_attribs);
			$this->set_tagline($tagline);
		}
		
		// ID
		if (preg_match('/<id>([^<]*)<\/id>/is', $xml, $id)) {
			$this->set_id($id[1]);
		}
		
		// INFO
		if (preg_match('/<info([^>]+)>(.*)<\/info>/is', $xml, $info)) {
			$info_attribs = $this->extract_attribs($info[1]);
			$info = array('info'=>$info[2]);
			$info = array_merge_recursive($info, $info_attribs);
			$this->set_info($info);
		}
		
		// MODIFIED
		if (preg_match('/<modified>([^<]*)<\/modified>/is', $xml, $modified)) {
			$this->set_modified($modified[1]);
		}
		
		// GENERATOR
		if (preg_match_all('/<generator([^>]*)>(.*)<\/generator>/sUi', $xml, $generator)) {
			$generator_attribs = $this->extract_attribs($generator[1][0]);
			$generator = array('generator'=>$generator[2][0]);
			$generator = array_merge_recursive($generator, $generator_attribs);
			$this->set_generator($generator);
		}
		
		// LINKS
		if (preg_match_all('/<link([^>]+)>/Ui', $xml, $links)) {
			foreach ($links[1] as $link) {
				$link = $this->extract_attribs($link);
				$this->add_link($link['href'], $link['rel'], $link['title'], $link['type']);
			}
		}
		
		// Handle all of the entries, creating AtomEntry objects and linking them
		preg_match_all('/(<entry[^>]*>.*<\/entry>)/sUi', $orig_xml, $entries_raw);
		if (strlen($entries_raw[0][0])) {
			foreach ($entries_raw[1] as $e=>$entry) {
				$ae = new AtomEntry();
				$ae->from_xml($entry);
				if ($ae) {
					$this->add_entry($ae);
				}
			}
		}
	}
	
	/**
	* @return String
	* @desc Returns an XML/String representation of the entire AtomFeed, including header values and all AtomEntries found in $this->entries.
	*/
	function to_xml() {
		$xml = '<?xml version="1.0" encoding="utf-8"?>' . "\n\n";
		
		$xml .= '<feed';
		
		// VERSION
		$ver = $this->get_version();
		if ($ver) {
			$xml .= ' version="' . $ver . '"';
		}
		
		$xml .= ' xmlns="http://purl.org/atom/ns#">' . "\n";
		
		// LINKS
		$links = $this->get_links();
		if (sizeof($links)) {
			foreach ($links as $link) {
				$xml .= '<link';
				foreach ($link as $attrib=>$val) {
					 if (strlen($attrib) && strlen($val)) {
					 	$xml .= ' ' . $attrib . '="' . utf8_encode($val) . '"';
					 }
				}
				$xml .= '/>' . "\n";
			}
		}
		
		// TITLE
		$title = $this->get_title();
		if (sizeof($title)) {
			$xml .= '<title';
			foreach ($title as $key=>$val) {
				if ($key != 'title') {
					$xml .= ' ' . $key . '="' . $val . '"';
				}
			}
			$xml .= '>' . utf8_encode($title['title']) . '</title>' . "\n";
		}
		
		// TAGLINE
		$tag = $this->get_tagline();
		if (sizeof($tag)) {
			$xml .= '<tagline';
			foreach ($tag as $key=>$val) {
				if ($key != 'tagline') {
					$xml .= ' ' . $key . '="' . $val . '"';
				}
			}
			$xml .= '>' . utf8_encode($tag['tagline']) . '</tagline>' . "\n";
		}
		
		// ID
		$id = $this->get_id();
		if ($id) {
			$xml .= '<id>' . $id . '</id>' . "\n";
		}
		
		// MODIFIED
		$mod = $this->get_modified();
		if ($mod) {
			$xml .= '<modified>' . $mod . '</modified>' . "\n";
		}
		
		
		// GENERATOR
		$gen = $this->get_generator();
		if (is_array($gen) && sizeof($gen)) {
			$xml .= '<generator url="' . $gen['url'] . '" version="' . $gen['version'] . '">' . $gen['generator'] . '</generator>' . "\n";
		}
		
		// INFO
		$info = $this->get_info();
		if (sizeof($info)) {
			$xml .= '<info';
			foreach ($info as $key=>$val) {
				if ($key != 'info') {
					$xml .= ' ' . $key . '="' . $val . '"';
				}
			}
			$xml .= '>' . utf8_encode($info['info']) . '</info>' . "\n";
		}
		
		// ENTRIES
		$entries = $this->get_entries();
		if ($entries) {
			foreach ($entries as $entry) {
				$xml .= $entry->to_xml('FEED');
			}
		}
		
		$xml .= '</feed>';
		
		return $xml;
	}
	
	/**
	* @return Int/FALSE
	* @desc Returns the last error registered in this class, or FALSE if none
	*/
	function error() {
		if (isset($this->err_no) && is_int($this->err_no)) {
			return $this->err_no;
		}
		else {
			return false;
		}
	}	
}



class AtomEntry {
	var $links = array();
	var $title = array();
	var $content = array();
	var $author = array();
	var $generator = array();
	var $issued;
	var $modified;
	var $created;
	var $id;
	var $err_no;
	
	/**
	* @return AtomEntry
	* @param Array $title
	* @param Array $content
	* @param Array $author
	* @param String $issued
	* @param String $modified
	* @param String $created
	* @desc Creates an AtomEntry, with optional initial values.
	*/
	function AtomEntry($title = array(), $content = array()) {
		$this->set_title($title);
		$this->set_content($content);
		
		// Default timezone is that of the current server (for dates)
		$this->tz = date('O');
	}
	
	/**
	* @return void
	* @param Array $title
	* @desc Sets the title details for the Entry, including some defaults if ommitted. FORMAT: array('title'=>$title);
	*/
	function set_title($title) {
		if (is_array($title)) {
			foreach ($title as $key=>$val) {
				if (strlen($key) && strlen($val)) {
					$this->title[$key] = $val;
				}
			}
		}
		else if (is_string($title)) {
			$this->title['title'] = $title;
			$this->title['mode']  = 'escaped';
		}
	}
	
	/**
	* @return String/Array/FALSE
	* @param String $elem
	* @desc Returns either the requested value, or all details of the $title
	*/
	function get_title($elem = false) {
		if ($elem === false) {
			return $this->title;
		}
		else {
			if (in_array($elem, array_keys($this->title))) {
				return $this->title[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param Array $content
	* @desc Sets the content details for the Entry, including some defaults if ommitted. FORMAT: array('content'=>$content); Also removes default <DIV> from Blogger entries to avoid PUT problems
	*/
	function set_content($content) {
		if (is_array($content)) {
			foreach ($content as $key=>$val) {
				$this->content[$key] = $val;
			}
		}
		else if (is_string($content)) {
			$this->content['content'] = $content;
			$this->content['mode']    = 'escaped';
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @param String $elem
	* @desc Returns the contents of this entry - including attributes of the content element
	*/
	function get_content($elem = false) {
		if ($elem === false) {
			return $this->content;
		}
		else {
			if (in_array($elem, array_keys($this->content))) {
				return $this->content[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param Array $author
	* @desc Set all details relating to the author (name, email, url)
	*/
	function set_author($author) {
		if (is_array($author)) {
			foreach ($author as $key=>$val) {
				$this->author[$key] = $val;
			}
		}
		else if (is_string($author)) {
			$this->author['name'] = $author;
		}
	}
	
	/**
	* @return Array/String/FALSE
	* @param String $elem
	* @desc Returns author details - either item requested, or all.
	*/
	function get_author($elem = false) {
		if ($elem === false) {
			return $this->author;
		}
		else {
			if (in_array($elem, array_keys($this->author))) {
				return $this->author[$elem];
			}
			else {
				return false;
			}
		}
	}
	
	/**
	* @return void
	* @param String $date
	* @desc Set the <issued> date, after sanitization
	*/
	function set_issued($date) {
		$this->issued = $this->sanitize_date($date);
	}
	
	/**
	* @return String
	* @desc Returns the <issued> date for the entry
	*/
	function get_issued() {
		if (isset($this->issued)) {
			return $this->issued;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $date
	* @desc Set the <modified> date, after sanitization
	*/
	function set_modified($date) {
		$this->modified = $this->sanitize_date($date);
	}
	
	/**
	* @return String/FALSE
	* @desc Returns the <modified> date for the entry
	*/
	function get_modified() {
		if (isset($this->modified)) {
			return $this->modified;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $date
	* @desc Set the <created> date, after sanitization
	*/
	function set_created($date) {
		$this->created = $this->sanitize_date($date);
	}
	
	/**
	* @return String/FALSE
	* @desc Returns the <created> date for the entry
	*/
	function get_created() {
		if (isset($this->created)) {
			return $this->created;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $id
	* @desc Set the unique <id> for the Entry
	*/
	function set_id($id) {
		$this->id = $id;
	}
	
	/**
	* @return String/FALSE
	* @desc Returns the unique <id> for this Entry
	*/
	function get_id() {
		if (isset($this->id)) {
			return $this->id;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $href
	* @param String $rel
	* @param String $title
	* @param String $type
	* @desc Adds a <link> to the array for this Entry. href, type, title and rel are all required
	*/
	function add_link($href, $rel, $title, $type) {
		if (strlen($href) && strlen($rel) && strlen($type)) {
			$this->links[] = array('href'=>$href, 'rel'=>$rel, 'title'=>$title, 'type'=>$type);
		}
	}
	
	/**
	* @return Array
	* @param String $key
	* @param String $val
	* @desc Returns the links where $key matches $val if specified, or all links if both ommitted
	*/
	function get_links($key = false, $val = false) {
		// If key and val are specified, then filter the array for entries where the specified key == the value
		if ($key !== false && $val !== false) {
			$out = array();
			foreach ($this->links as $link) {
				if ($link[$key] == $val) {
					$out[] = $link;
				}
			}
		}
		else {
			$out = $this->links;
		}
		
		return $out;
	}
	
	function sanitize_date($date) {
		if ($date == '' || $date === false) {
			return false;
		}
		else if (!stristr($date, 'T') || !stristr($date, 'Z')) {
			if (preg_match('/(\d{4})-(\d{2})-(\d{2}).(\d{2}):(\d{2}):(\d{2})-(\d{2}):(\d{2})/', $date, $parts)) {
				return $date;
			}
			else if (preg_match('/(\d{4})-(\d{2})-(\d{2}).(\d{2}):(\d{2}):(\d{2})/', $date, $parts)) {
				return $date;
			}
			return false;
		}
		else {
			return $date;
		}
	}
	
	function atom_date($date, $tz) {
		if ($date == '' || $date === false) {
			preg_match('/(\+|-)(\d{2}):(\d{2})/', $tz, $tz_parts);
			return gmdate('Y-m-d\TH:i:s\Z', mktime(eval("date('H') " . $tz_parts[1] . " " . $tz_parts[2] . ";"), eval("date('i') " . $tz_parts[1] . " " . $tz_parts[3] . ";"), date('s'), date('m'), date('d'), date('Y')));
		}
		else {
			return $date;
		}
	}
	
	/**
	* @return Array
	* @param String $str
	* @desc Parses an XML/HTML string and returns an associative array containing key/value pairs
	*/
	function extract_attribs($str) {
		$out = array();
		if (preg_match_all('/([^ =]+)="([^"]*)"/si', $str, $attribs)) {
			foreach ($attribs[1] as $c=>$key) {
				$out[$key] = $attribs[2][$c];
			}
		}
				
		return $out;
	}
	
	/**
	* @return void
	* @param String $xml
	* @desc Builds out the details of the object by parsing an XML <entry>
	*/
	function from_xml($xml) {
		// Parse an XML <entry> into its elements and then set them to this Object
		// LINKS
		if (preg_match_all('/<link([^>]+)>/Ui', $xml, $links)) {
			foreach ($links[1] as $link) {
				$link = $this->extract_attribs($link);
				$this->add_link($link['href'], $link['rel'], $link['title'], $link['type']);
			}
		}
		
		// AUTHOR----------------------------------------------------------------------------------------------*
		// Needs to handle the other elements properly - but need a sample to work from!
		if (preg_match('/<author>\s*(<([^>]+)>(.*)<\/\2>)*\s*<\/author>/sUi', $xml, $author)) {
			$this->set_author(array($author[2]=>$author[3]));
		}
		
		// DATES
		if (preg_match('/<issued>([^>]*)<\/issued>/i', $xml, $date)) {
			$this->set_issued($date[1]);
		}
		
		if (preg_match('/<modified>([^>]*)<\/modified>/i', $xml, $date)) {
			$this->set_modified($date[1]);
		}
		
		if (preg_match('/<created>([^>]*)<\/created>/i', $xml, $date)) {
			$this->set_created($date[1]);
		}
		
		// ID
		if (preg_match('/<id>([^>]*)<\/id>/i', $xml, $id)) {
			$this->set_id($id[1]);
		}
		
		// TITLE
		if (preg_match_all('/<title([^>]*)>(.*)<\/title>/sUi', $xml, $title)) {
			$title_attribs = $this->extract_attribs($title[1][0]);
			$title = array('title'=>$title[2][0]);
			$title = array_merge_recursive($title, $title_attribs);
			$this->set_title($title);
		}
		
		// CONTENT
		if (preg_match_all('/<content([^>]*)>(.*)<\/content>/sUi', $xml, $content)) {
			$content_attribs = $this->extract_attribs($content[1][0]);
			$content = array('content'=>$content[2][0]);
			$content = array_merge_recursive($content, $content_attribs);
			$this->set_content($content);
		}
	}
	
	/**
	* @return String
	* @param String $purpose
	* @desc Creates an XML representation of this Entry. $purpose is [PUT|POST|FEED] and defines what you're going to do with this XML. $purpose modifies the elements included according to the Atom spec.
	*/
	function to_xml($purpose = 'POST') {
		$xml = '';
		
		// XML prolog if PUT or POST
		if ($purpose == 'PUT' || $purpose == 'POST') {
			$xml .= '<?xml version="1.0" encoding="utf-8"?>' . "\n";
		}

		// Start XML packet
		$xml .= '<entry xmlns="http://purl.org/atom/ns#">' . "\n";
		
		// GENERATOR
		if ($purpose == 'PUT' || $purpose == 'POST') {
			$xml .= '<generator version="0.1" url="http://www.dentedreality.com.au/phpatomapi/">Dented Reality PHP Atom API</generator>' . "\n";
		}
		
		// LINKS
		$links = $this->get_links();
		if (sizeof($links)) {
			foreach ($links as $link) {
				$xml .= '<link';
				foreach ($link as $attrib=>$val) {
					 if (strlen($attrib) && strlen($val)) {
					 	$xml .= ' ' . $attrib . '="' . utf8_encode($val) . '"';
					 }
				}
				$xml .= '/>' . "\n";
			}
		}
		
		// AUTHOR
		$author = $this->get_author();
		if (sizeof($author)) {
			$xml .= '<author>';
			foreach ($author as $key=>$val) {
				if (strlen($key) && strlen($val)) {
					$xml .= '<' . $key .'>' . utf8_encode($val) . '</' . $key . '>' . "\n";
				}
			}
			$xml .= '</author>' . "\n";
		}
		
		// DATES (issued, created, modified)
		if (strlen($this->get_issued())) {
			$xml .= '<issued>' . $this->get_issued() . '</issued>' . "\n";
		}
		
		// Modified and Id, only if not creating a POST entry
		if ($purpose != 'POST' && $purpose != 'PUT') {
			if (strlen($this->get_created())) {
				$xml .= '<created>' . $this->get_created() . '</created>' . "\n";
			}
			if (strlen($this->get_modified())) {
				$xml .= '<modified>' . $this->get_modified() . '</modified>' . "\n";
			}
			
			// ID element
			$xml .= '<id>' . $this->id . '</id>' . "\n";
		}
		
		// TITLE
		$title = $this->get_title();
		if (is_array($title)) {
			$xml .= '<title';
			foreach ($title as $key=>$val) {
				if ($key != 'title') {
					$xml .= ' ' . $key . '="' . $val . '"';
				}
			}
			$xml .= '>' . utf8_encode($title['title']) . '</title>' . "\n";
		}
		
		// CONTENT
		$content = $this->get_content();
		if (is_array($content)) {
			$xml .= '<content';
			foreach ($content as $key=>$val) {
				if ($key != 'content') {
					$xml .= ' ' . $key . '="' . $val . '"';
				}
			}
			$xml .= '>' . utf8_encode($content['content']) . '</content>' . "\n";
		}
		
		$xml .= '</entry>' . "\n\n";
		
		return $xml;
	}
	
	/**
	* @return Int/FALSE
	* @desc Returns the last error registered in this class, or FALSE if none
	*/
	function error() {
		if (isset($this->err_no) && is_int($this->err_no)) {
			return $this->err_no;
		}
		else {
			return false;
		}
	}
}



class AtomRequest {
	var $method;
	var $uri;
	var $auth;
	var $payload;
	var $response;
	var $httpcode;
	var $err_no;
	
	/**
	* @return AtomRequest
	* @param String $method
	* @param String $uri
	* @param AuthObject $auth
	* @param String $payload
	* @desc Creates an object with functions for performing standard Atom requests. $method should be [GET|PUT|POST|DELETE], $uri is the endpoint/uri to execute the request against, $auth is an authentication object (BasicAuth or WSSE) and $payload is the XML of the request, if required (POST|PUT)
	*/
	function AtomRequest($method, $uri, $auth, $payload = false) {
		$this->set_method($method);
		$this->set_uri($uri);
		$this->set_auth($auth);
		$this->set_payload($payload);
		
		if ($this->error()) {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $uri
	* @desc Set the URI to execute this AtomRequest against. Should be a fully-qualified URL and start with 'http'
	*/
	function set_uri($uri) {
		if (substr($uri, 0, 4) == 'http') {
			$this->uri = $uri;
		}
		else {
			$this->err_no = ATOMAPI_ENDPOINT_INVALID;
		}
	}
	
	/**
	* @return String/FALSE on error
	* @desc Returns the URI stored for this AtomRequest to operate against, or FALSE if not set yet
	*/
	function get_uri() {
		if (isset($this->uri) && strlen($this->uri)) {
			return $this->uri;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param AuthObj $auth
	* @desc Set the authentication object to use for this AtomRequest
	*/
	function set_auth($auth) {
		if (is_object($auth)) {
			$this->auth = $auth;
		}
		else {
			$this->err_no = ATOMAPI_AUTHTYPE_INVALID;
		}
	}
	
	/**
	* @return void
	* @param String $method
	* @desc Sets the method of this AtomRequest. Allowed values are [GET|PUT|POST|DELETE]
	*/
	function set_method($method) {
		if (in_array($method, array('GET', 'PUT', 'POST', 'DELETE'))) {
			$this->method = $method;
		}
		else {
			$this->err_no = ATOMAPI_METHOD_INVALID;
		}
	}
	
	/**
	* @return String/FALSE on error
	* @desc Returns the current $method set for this AtomRequest. Should be [GET|PUT|POST|DELETE]. Returns FALSE if it is unset.
	*/
	function get_method() {
		if (isset($this->method) && strlen($this->method)) {
			return $this->method;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param String $xml
	* @desc Set the payload (body) of the AtomRequest. Should be an XML string, and is only required for POST and PUT operations
	*/
	function set_payload($xml) {
		if (is_string($xml)) {
			$this->payload = $xml;
		}
		else {
			$this->err_no = ATOMAPI_PAYLOAD_INVALID;
		}
	}
	
	/**
	* @return String/FALSE on error
	* @desc Returns the current stored payload, which should be an XML package, or FALSE if it is not set yet.
	*/
	function get_payload() {
		if (isset($this->payload)) {
			return $this->payload;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return Int/FALSE on error
	* @desc Executes the AtomRequest on the URI specified in the object. Returns the HTTP response code, or FALSE on a serious error
	*/
	function exec() {
		// Confirm minimum requirements (method, uri)
		if (!$this->get_uri() || !$this->get_method()) {
			$this->err_no = ATOMAPI_REQUEST_INVALID;
			return;
		}
		
		// Common cURL configuration directives
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $this->get_uri());
		curl_setopt($ch, CURLOPT_USERAGENT, "Dented Reality PHP Atom API Library v0.1");
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0); // Don't stress about SSL validity
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); // Return the response, don't output it
		
		// Handle configuration of specific options for certain request types
		switch ($this->get_method()) {
			case 'POST' :
				// Authentication header and content-type
				$headers = $this->auth->get_header(true);
				$headers[] = 'Content-type: application/xml';
				curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
				
				// Configuring post contents (should be an XML payload)
				curl_setopt($ch, CURLOPT_POST, 1);
				curl_setopt($ch, CURLOPT_POSTFIELDSIZE, strlen($this->get_payload()));
				curl_setopt($ch, CURLOPT_POSTFIELDS, $this->get_payload());
				break;
			case 'PUT' :
				// PUT requires the payload to be written to file, and passed as a file pointer
				$put = tmpfile();
				fputs($put, $this->get_payload());
				rewind($put);
				
				// Authentication headers and content-type
				$headers = $this->auth->get_header(true);
				$headers[] = 'Content-type: application/xml';
				curl_setopt($ch, CURLOPT_HTTPHEADER, $headers); 
				
				// Performing a PUT operation, requires file pointer and size of payload
				curl_setopt($ch, CURLOPT_PUT, 1);
				curl_setopt($ch, CURLOPT_INFILE, $put);
				curl_setopt($ch, CURLOPT_INFILESIZE, strlen($this->get_payload()));
				break;
			case 'DELETE' :
				// Simple DELETE request (authenticated)
				curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
				curl_setopt($ch, CURLOPT_HTTPHEADER, $this->auth->get_header(true));
				break;
			case 'GET' :
			default :
				// Straight GET, with authentication headers
				curl_setopt($ch, CURLOPT_HTTPHEADER, $this->auth->get_header(true));
		}
		
		// Execute cURL session and handle results
		$this->response = curl_exec($ch);
		
		if (curl_errno($ch)) {
			$this->err_no = ATOMAPI_CURL_ERROR;
			curl_close($ch);
			return;
		}
		else {
			// Set the HTTPcode of the response into this object, then return it
			$this->set_httpcode(curl_getinfo($ch, CURLINFO_HTTP_CODE));
			curl_close($ch);
			return $this->get_httpcode();
		}
	}
	
	/**
	* @return String/FALSE on error
	* @desc Returns the response content from the exec()'d AtomRequest, or FALSE if not set yet
	*/
	function get_response() {
		if (isset($this->response)) {
			return $this->response;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return void
	* @param Int $code
	* @desc Internal function, used to update the http status code returned when the AtomRequest is exec()'d
	*/
	function set_httpcode($code) {
		if (strlen($code) && is_int($code)) {
			$this->httpcode = $code;
		}
		else {
			$this->httpcode = false;
		}
	}
	
	/**
	* @return Int/FALSE on error
	* @desc Returns the last stored http status code for this AtomRequest
	*/
	function get_httpcode() {
		if (isset($this->httpcode) && strlen($this->httpcode)) {
			return $this->httpcode;
		}
		else {
			return false;
		}
	}
	
	/**
	* @return Int/FALSE
	* @desc Returns the last error registered in this class, or FALSE if none
	*/
	function error() {
		if (isset($this->err_no) && is_int($this->err_no)) {
			return $this->err_no;
		}
		else {
			return false;
		}
	}	
}

?>