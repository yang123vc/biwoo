


CREATE DATABASE biwoo;

USE biwoo;

CREATE TABLE userinfo_t
(
	account 	VARCHAR(32)	PRIMARY KEY,
	password 	VARCHAR(16),
	username 	VARCHAR(32),
	nick 			VARCHAR(32),
	gender 		TINYINT DEFAULT 0,
	birthday 	DATE,
	phone			VARCHAR(32),
	email			VARCHAR(32)
);

CREATE TABLE systemsetting_t
(
	curdialogid	INT
);

CREATE TABLE accountsetting_t
(
	account 		VARCHAR(32)	PRIMARY KEY,
	curgroupid	INT UNSIGNED,
	curdataid		INT UNSIGNED,
	curdatagroupid	INT UNSIGNED
);

CREATE TABLE groupinfo_t
(
	account 		VARCHAR(32)	PRIMARY KEY,
	groupid			INT UNSIGNED,
	parentgroupid	INT UNSIGNED,
	groupname		VARCHAR(32),
	grouptype		TINYINT
);

CREATE TABLE friendinfo_t
(
	account 		VARCHAR(32)	PRIMARY KEY,
	groupid			INT UNSIGNED,
	substate		TINYINT,
	note				VARCHAR(255)
);

CREATE TABLE company_t
(
	coid				INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	name				VARCHAR(255) NOT NULL,
	createdate	DATE

);

CREATE TABLE cogroup_t
(
	groupid				INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	parentgroupid	INT UNSIGNED DEFAULT 0,
	coid					INT UNSIGNED,
	name					VARCHAR(255)	NOT NULL,
	type					TINYINT
);

// userinfo_t.account cogroup_t.groupid TABLE;
CREATE TABLE usercogroup_t
(
	account 			VARCHAR(32) NOT NULL,
	groupid				INT UNSIGNED
	
);

CREATE TABLE offevents_t
(
	id							INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	event						INT,
	fromtype				TINYINT DEFAULT 0,
	fromid					INT DEFAULT 0,
	fromaccount			VARCHAR(32) NOT NULL,
	toaccount				VARCHAR(32) NOT NULL,
	msgtype					SMALLINT DEFAULT 1,
	newflag					BOOLEAN DEFAULT true,
	message					VARCHAR(6000),
	msgsize					INT UNSIGNED DEFAULT 0,
	width						SMALLINT UNSIGNED DEFAULT 0,
	height					SMALLINT UNSIGNED DEFAULT 0,
	msgtime					TIMESTAMP DEFAULT CURRENT_TIMESTAMP

);
