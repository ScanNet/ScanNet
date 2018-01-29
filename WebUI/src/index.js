'use strict';

const app = require('./app');
const port = app.get('port');
const server = app.listen(port);

app.use(app.logger.expressLogger);

app.logger.info('Logging...');

process.on('unhandledRejection', (reason, p) => {
  console.log("Unhandled Rejection at: Promise ", p, " reason: ", reason);
});

server.on('listening', () =>
  console.log(`Feathers application started on ${app.get('host')}:${port}`)
);
