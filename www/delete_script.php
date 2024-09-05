<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $filename = $_POST['filename'];
    $filePath = 'path/to/your/directory/' . $filename;

    if (file_exists($filePath)) {
        unlink($filePath); // Delete the file
        echo "File '$filename' deleted successfully.";
    } else {
        echo "File '$filename' does not exist.";
    }
}
?>