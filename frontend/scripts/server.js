const express = require('express')
const path = require('path')
const app = express();

const frontendDir = path.join(__dirname, '..');
app.set('views', frontendDir);

app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');

app.use(express.static(frontendDir));

app.get('/', (req, res) => {res.render('index')})

app.listen(6969)
