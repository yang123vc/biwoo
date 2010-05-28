


CREATE DATABASE bocinfo;

USE bocinfo;

CREATE TABLE loginrecords_t
(
	account		VARCHAR(32) PRIMARY KEY,
	password	VARCHAR(32)
);

CREATE TABLE bocinfo_t
(
	lastaccount		VARCHAR(32) NOT NULL,
	autologin			BOOLEAN

);

