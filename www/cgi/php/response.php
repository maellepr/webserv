<?php
    $query = $_SERVER["QUERY_STRING"];
    $params = array();
    parse_str($query, $params);

    echo '<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link href="/style/style.css" rel="stylesheet">
        <link href="/style/response.css" rel="stylesheet">
        <title>Response</title>
    </head>
    <body>
        <h1 class="pagesTitle">Response</h1>
        <div class="response">
            <div class="tag">Your favorite color is <span class="value">' .$params['color'] . '</span></div>
            <div class="tag">Your favorite meal is <span class="value">' .$params['meal'] . '</span></div>
            <div class="tag">Your favorite season <span class="value">' .$params['season'] . '</span></div>
        </div>
        <div class="index">
            <a class="indexButton" href="/site_index.html">Index</a>
        </div>
    </body>
    </html>';
?>
