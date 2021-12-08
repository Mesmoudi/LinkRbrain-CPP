DROP DATABASE IF EXISTS linkrbrain2019;
DROP USER IF EXISTS linkrbrain_user;

CREATE DATABASE linkrbrain2019;
CREATE USER linkrbrain_user WITH ENCRYPTED PASSWORD 'linkrbrain_password';
GRANT ALL PRIVILEGES ON DATABASE linkrbrain2019 TO linkrbrain_user;

\c linkrbrain2019

CREATE TABLE queries (
    id SERIAL,
    name VARCHAR(16),
    settings JSONB,
    groups JSONB
);
ALTER TABLE queries OWNER TO linkrbrain_user;
