const express = require('express')
const path = require('path')
const app = express();

const frontendDir = path.join(__dirname, '..');
app.set('views', frontendDir);

app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');

app.use(express.static(frontendDir));

app.get('/', (req, res) => {res.render('index')})

const firstPort = Number(process.env.PORT) || 3000;
const lastPort = firstPort + 20;

function listen(port) {
	const server = app.listen(port, () => {
		console.log(`Serving app at http://localhost:${port}/`);
	});

	server.on('error', (error) => {
		if (error.code === 'EADDRINUSE' && port < lastPort) {
			listen(port + 1);
			return;
		}

		console.error(`Could not start server on ports ${firstPort}-${lastPort}:`, error.message);
		process.exitCode = 1;
	});
}

listen(firstPort);
