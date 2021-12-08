CREATE DATABASE linkrbrain2019;
CREATE USER linkrbrain_user WITH ENCRYPTED PASSWORD 'linkrbrain_password';
GRANT ALL PRIVILEGES ON DATABASE linkrbrain2019 TO linkrbrain_user;

\c linkrbrain2019

CREATE TABLE datasets (
    id SERIAL,
    name VARCHAR(16),
    label VARCHAR(64),
    metadata JSONB
);
ALTER TABLE datasets OWNER TO linkrbrain_user;

CREATE TABLE tests (
    id SERIAL,
    name VARCHAR(16)
);
ALTER TABLE tests OWNER TO linkrbrain_user;
