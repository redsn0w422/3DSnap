<?php
$url1 = $_GET['url1'];
$url2 = $_GET['url2'];

$im = new Imagick();

$im1 = new Imagick($url1);
$im2 = new Imagick($url2);

$im->addImage($im1);
$im->addImage($im2);

$im->resetIterator();
$combined = $im->appendImages(false);

$combined->setImageFormat('jpg');
header('Content-Type: image/jpg');
echo $combined;
?>
