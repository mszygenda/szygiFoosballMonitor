const express = require('express')
const bodyParser = require('body-parser');
const vibrationService = require('./vibrationService');

const app = express()
const port = 3000

app.use(bodyParser.json());
app.get('/status', (req, res) => {
  res.send(JSON.stringify({
    isVibrating: vibrationService.isVibrating(),
    lastPosition: vibrationService.getLastPosition()
  }));
});

app.post('/position', (req, res) => {
  const { value } = req.body;

  vibrationService.addPosition(value);

  res.send('OK')
});

app.listen(port, () => console.log(`Example app listening on port ${port}!`))