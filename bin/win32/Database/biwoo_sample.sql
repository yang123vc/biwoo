


CREATE DATABASE biwoo;

USE biwoo;

//////////////////////////////////////////;
// system setting;
INSERT INTO systemsetting_t(curdialogid) VALUES(0);


//////////////////////////////////////////;
// Create user account here;
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('aaa', 'aaa', 'UserAccount1', 1, '1001', '85510000', '137xx', 'aaa@company.com');
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('bbb', 'bbb', 'UserAccount2', 1, '1002', '85510000', '137xx', 'bbb@company.com');
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('ccc', 'ccc', 'UserAccount3', 1, '1003', '85510000', '137xx', 'ccc@company.com');
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('ddd', 'ddd', 'UserAccount4', 1, '1004', '85510000', '137xx', 'ddd@company.com');
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('eee', 'eee', 'UserAccount5', 2, '1005', '85510000', '137xx', 'eee@company.com');
INSERT INTO userinfo_t(account, password, username, gender, extension, phone, mobile, email) VALUES('fff', 'fff', 'UserAccount6', 2, '1006', '85510000', '137xx', 'fff@company.com');


////////////////////////////////////////////////////;
// Change the company name here;
INSERT INTO company_t(name) VALUES('Open Source Software Infermation Corporation');


/////////////////////////////////////////////////////;
// Create enterprise department(type=1) or project(type=2) info here;
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Administration', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Integrated Management', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Accounting Department', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Marketing Department', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Commerce Department', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Human Resource Department', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'System Integration Center', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Software Development Center', 1);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Cisco Project', 2);
INSERT INTO cogroup_t(coid, name, type) VALUES(1, 'Intel Project', 2);


//////////////////////////////////////////////////////;
// Setting enterprise organization here;
// 7 System Integration Center;
INSERT INTO cogroup_t(parentgroupid, coid, name, type) VALUES(7, 1, 'Departemnt 1', 1);
INSERT INTO cogroup_t(parentgroupid, coid, name, type) VALUES(7, 1, 'Department 2', 1);

// 8 Software Development Center;
INSERT INTO cogroup_t(parentgroupid, coid, name, type) VALUES(8, 1, 'Government & Enterprise Software', 1);
INSERT INTO cogroup_t(parentgroupid, coid, name, type) VALUES(8, 1, 'Mobile Software Department', 1);


//////////////////////////////////////////////////////;
// Setting the department or project user account;
// 9 Cisco Project;
INSERT INTO usercogroup_t(account, groupid) VALUES('aaa', 9);
INSERT INTO usercogroup_t(account, groupid) VALUES('bbb', 9);
INSERT INTO usercogroup_t(account, groupid) VALUES('ccc', 9);

// 10 Intel Project;
INSERT INTO usercogroup_t(account, groupid) VALUES('ddd', 10);
INSERT INTO usercogroup_t(account, groupid) VALUES('eee', 10);
INSERT INTO usercogroup_t(account, groupid) VALUES('fff', 10);

// 13 Government & Enterprise Software;
// 14 Mobile Software Department;
INSERT INTO usercogroup_t(account, groupid) VALUES('aaa', 13);
INSERT INTO usercogroup_t(account, groupid) VALUES('bbb', 13);
INSERT INTO usercogroup_t(account, groupid) VALUES('ccc', 13);
INSERT INTO usercogroup_t(account, groupid) VALUES('ddd', 13);
INSERT INTO usercogroup_t(account, groupid) VALUES('eee', 14);
INSERT INTO usercogroup_t(account, groupid) VALUES('fff', 14);


