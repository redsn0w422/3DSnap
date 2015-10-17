<?php
require_once("src/snapchat.php");

//////////// CONFIG ////////////
$username       = ""; // Your snapchat username
$password       = ""; // Your snapchat password
$gEmail         = ""; // Gmail account
$gPasswd        = ""; // Gmail account password
$casperKey      = "e353a611077d86617e6e5ffbb3a3774f"; // Casper API Key
$casperSecret   = "de518df5975757cf18143185dbd25cc6"; // Casper API Secret
$debug = true; // Set this to true if you want to see all outgoing requests and responses from server
////////////////////////////////

$username = $_POST['username'];
$password = $_POST['password'];
$gEmail = $_POST['gEmail'];
$gPasswd = $_POST['gPasswd'];

echo $username;
echo $password;
echo $gEmail;
echo $gPasswd;


$imagePath = $_POST['imgUrl']; // URL or local path to a media file (image or video)

$sendTo = array();

$snapchat = new Snapchat($username, $gEmail, $gPasswd, $casperKey, $casperSecret, $debug);

//Login to Snapchat with your username and password
$snapchat->login($password);

// Get your friends in an array
$friends = $snapchat->getFriends();

echo "My friends: ";
echo $friends;


// Send snap adding text to your image and 10 seconds
// $snapchat->send($imagePath, $sendTo, "this is a test :D", 10);

// Set a story
// $snapchat->setStory($imagePath);

// Set a story adding text to the image and 5 seconds
// $snapchat->setStory($imagePath, 5, "This is my story");


// Get snaps data (Without storing them)
//$snapchat->getSnaps();

// Automatically downloads Snaps and store it in 'Snaps' folder
// $snapchat->getSnaps(true);

// Download stories for a specific username
// $snapchat->getStoriesByUsername("homie", true);

// Send chat message to "username"
// $snapchat->sendMessage("redsn0w422", "hello from Snap-API!");

$snapchat->closeAppEvent();

?>
