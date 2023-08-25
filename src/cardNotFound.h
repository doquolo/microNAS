const char cardNotFound[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <link rel="shortcut icon" href="server-solid.svg" type="image/x-icon" />
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #202124;
      color: #ffffff;
    }

    header {
      background-color: #1a1a1a;
      color: #ffffff;
      padding: 10px 20px;
    }

    header h1 {
      margin: 0;
    }

    .file-list {
      padding: 20px;
    }

    .file {
      display: flex;
      flex-direction: column;
      align-items: flex-start;
      border-bottom: 1px solid #303030;
      padding: 10px 0;
    }

    .file svg {
      width: 40px;
      height: 40px;
      margin-right: 10px;
      fill: white;
    }

    .file > p, h1 {
        margin: 0rem 0rem 1rem 0rem;
    }
  </style>
  <title>microNAS - Initial Setup</title>
</head>
<body>
  <header>
    <h1 id="title">Error - microNAS</h1>
  </header>
  <div class="file-list">
    <p>Card initialization failed. Things to check: </p>
    <p>1. is a card inserted?</p>
    <p>2. is your wiring correct?</p>
    <p>3. did you change the chipSelect pin to match your shield or module?</p>
    <p>Note: press reset after fixing your issue!</p>
  </div>
</body>
</html>
)rawliteral";