
CREATE DATABASE bocdb;

USE bocdb;

CREATE TABLE accountconversations_t
(
	fromtype				TINYINT DEFAULT 0,
	fromid					INT DEFAULT 0,
	fromaccount			VARCHAR(32),
	sayaccount			VARCHAR(32),
	msgtype					SMALLINT DEFAULT 1,
	subtype					SMALLINT DEFAULT 1,
	newflag					BOOLEAN DEFAULT true,
	message					VARCHAR(6000),
	msgsize					INT UNSIGNED DEFAULT 0,
	width						SMALLINT UNSIGNED DEFAULT 0,
	height					SMALLINT UNSIGNED DEFAULT 0,
	msgtime					TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE historydialog_t
(
	dialogid				INT,
	account					VARCHAR(32),
	manager					BOOLEAN
);
