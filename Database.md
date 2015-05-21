**[Index](Index.md)**

---

# 1 bodb #
Biwoo use **bodb** library save all data, including the organizational structure,
user information and the client chatting record, etc.

Bodb use **bodbclient** application to generate biwoo database files.

  * **Windows:** $(BIWOO\_PATH)\bin\win32\Database\bodbclient.exe
  * **Linux:** $(BIWOO\_PATH)\bin\linux\Database\bodbclient

## 1.1 Bodbclient usage ##
**Usage:**
```
BODB:EXEC [SQL_FILE];
BODB:[SQL];
```

**Sample:**
```
BODB:EXEC biwoo.sql;
BODB:SELECT * FROM user_t WHERE account='aaa';
```

# 2 Biwoo database files #
## 2.1 Server database SQL file ##
  * **Windows:** $(BIWOO\_PATH)\bin\win32\Database\biwoo.sql
  * **Linux:** $(BIWOO\_PATH)\bin\linux\Database\biwoo.sql

  1. Enterprise information table;
  1. Department and project group table;
  1. User account information table;
  1. System setting table;
  1. Account offline information event table;
  1. etc;

**Please don't modify biwoo.sql this file.**

## 2.2 Server sample SQL file ##
  * **Windows:** $(BIWOO\_PATH)\bin\win32\Database\biwoo\_sample.sql
  * **Linux:** $(BIWOO\_PATH)\bin\linux\Database\biwoo\_sample.sql

**Please edit biwoo\_sample.sql this file, add, delete, or modify department, user account information.**

## 2.3 Server database file ##
  * **Windows:** $(BIWOO\_PATH)\bin\win32\BiwooServer\conf\biwoo\db\biwoo\biwoo.bdf
  * **Linux:** $(BIWOO\_PATH)\bin\linux\BiwooServer\conf\biwoo\db\biwoo\biwoo.bdf

**Generate biwoo database file:**

  * Edit biwoo\_sample.sql file;
  * Run **bodbclient** application, and perform the following commands:
```
BODB:EXEC biwoo.sql;
BODB:EXEC biwoo_sample.sql;
```
  * Replace the biwoo\biwoo.bdf file.

## 2.4 Client database file ##
  * **SQL file:** $(BIWOO\_PATH)\bin\win32\Database\bocdb.sql
  * **Database file:** $(BIWOO\_PATH)\bin\win32\BiwooClient\default\bocdb\bocdb.bdf

  * User chat information record table.

**Please don't modify bocdb.sql this file.**