/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

process.env.UV_THREADPOOL_SIZE = 2;

const express = require('express');
const rateLimit = require('express-rate-limit')
const cors = require('cors');

const servers = require('./routes/api/v1/servers');

const config = require("./config/config.json")

const app = express();

const limiter = rateLimit({
	windowMs: 1 * 60 * 1000, 
	max: 300, 
	standardHeaders: true, 
	legacyHeaders: false,
});

app.use(cors());
app.use(express.json({}));
app.use(limiter);

app.get('/', (req, res) => { res.send('Please use the appropriate API\'s to access this service.'); });
app.use('/api/v1/servers', servers);

app.listen(config.port, () => { console.log(`This service is now listening on port ${config.port}!`); }); 
