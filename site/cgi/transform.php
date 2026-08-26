#!/usr/bin/env php-cgi
<?php
// 1. Start high-precision timer
$start_time = microtime(true);

// 2. Read input from GET or POST
$raw_input = "";
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

if ($method === 'POST') {
    $raw_input = $_POST['text'] ?? '';
    if (empty($raw_input)) {
        $body = file_get_contents("php://input");
        parse_str($body, $parsed);
        $raw_input = $parsed['text'] ?? '';
    }
} else {
    $raw_input = $_GET['text'] ?? '';
}

if (empty($raw_input)) {
    $raw_input = "Hello 42 Webserv!";
}

// 3. Perform Transformations
$sha256_res = hash('sha256', $raw_input);
$md5_res = md5($raw_input);
$base64_res = base64_encode($raw_input);

// Binary string
$binary_res = "";
for ($i = 0; $i < strlen($raw_input); $i++) {
    $binary_res .= sprintf("%08b ", ord($raw_input[$i]));
}

// 4. Stop timer
$elapsed_ms = (microtime(true) - $start_time) * 1000;

// 5. Output JSON response
$result = [
    "language" => "PHP",
    "input" => $raw_input,
    "time_ms" => round($elapsed_ms, 4),
    "sha256" => $sha256_res,
    "md5" => $md5_res,
    "base64" => $base64_res,
    "binary" => trim($binary_res)
];

header("Content-Type: application/json");
echo json_encode($result);
