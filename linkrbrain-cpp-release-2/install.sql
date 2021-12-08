-- Create new database

CREATE DATABASE __DBNAME__;

-- Create new database user if not already existing, and grant it the new database

DO $$
BEGIN
    CREATE USER __USER__ WITH ENCRYPTED PASSWORD '__PASSWORD__';
    EXCEPTION
        WHEN DUPLICATE_OBJECT THEN RAISE NOTICE 'not creating database user __USER__, already exists';
END
$$;
GRANT ALL PRIVILEGES ON DATABASE __DBNAME__ TO __USER__;

-- Connect to newly created database

\c __DBNAME__

-- Create new schema

-- CREATE SCHEMA IF NOT EXISTS __SCHEMA__;
ALTER DEFAULT PRIVILEGES IN SCHEMA __SCHEMA__ GRANT ALL PRIVILEGES ON TABLES TO __USER__;


--
-- PostgreSQL database dump
--

-- Dumped from database version 10.12 (Ubuntu 10.12-0ubuntu0.18.04.1)
-- Dumped by pg_dump version 10.12 (Ubuntu 10.12-0ubuntu0.18.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner:
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner:
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: queries; Type: TABLE; Schema: __SCHEMA__; Owner: __USER__
--

CREATE TABLE IF NOT EXISTS __SCHEMA__.queries (
    id integer NOT NULL,
    name character varying(64),
    settings jsonb NOT NULL,
    groups jsonb NOT NULL,
    permissions jsonb NOT NULL,
    created_at timestamp without time zone DEFAULT now() NOT NULL,
    deleted_at timestamp without time zone,
    updated_at timestamp without time zone DEFAULT now() NOT NULL,
    is_computed boolean DEFAULT false NOT NULL,
    correlations jsonb DEFAULT 'null'::jsonb NOT NULL,
    graph jsonb DEFAULT 'null'::jsonb NOT NULL
);


ALTER TABLE __SCHEMA__.queries OWNER TO __USER__;

--
-- Name: queries_id_seq; Type: SEQUENCE; Schema: __SCHEMA__; Owner: __USER__
--

CREATE SEQUENCE IF NOT EXISTS __SCHEMA__.queries_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE __SCHEMA__.queries_id_seq OWNER TO __USER__;

--
-- Name: queries_id_seq; Type: SEQUENCE OWNED BY; Schema: __SCHEMA__; Owner: __USER__
--

ALTER SEQUENCE __SCHEMA__.queries_id_seq OWNED BY __SCHEMA__.queries.id;


--
-- Name: users; Type: TABLE; Schema: __SCHEMA__; Owner: __USER__
--

CREATE TABLE IF NOT EXISTS __SCHEMA__.users (
    id integer NOT NULL,
    username character varying NOT NULL,
    password character varying NOT NULL,
    created_at timestamp without time zone DEFAULT now() NOT NULL
);


ALTER TABLE __SCHEMA__.users OWNER TO __USER__;

--
-- Name: users_id_seq; Type: SEQUENCE; Schema: __SCHEMA__; Owner: __USER__
--

CREATE SEQUENCE IF NOT EXISTS __SCHEMA__.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE __SCHEMA__.users_id_seq OWNER TO __USER__;

--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: __SCHEMA__; Owner: __USER__
--

ALTER SEQUENCE __SCHEMA__.users_id_seq OWNED BY __SCHEMA__.users.id;


--
-- Name: queries id; Type: DEFAULT; Schema: __SCHEMA__; Owner: __USER__
--

ALTER TABLE ONLY __SCHEMA__.queries ALTER COLUMN id SET DEFAULT nextval('__SCHEMA__.queries_id_seq'::regclass);


--
-- Name: users id; Type: DEFAULT; Schema: __SCHEMA__; Owner: __USER__
--

ALTER TABLE ONLY __SCHEMA__.users ALTER COLUMN id SET DEFAULT nextval('__SCHEMA__.users_id_seq'::regclass);


--
-- Name: users uniq_username_users; Type: CONSTRAINT; Schema: __SCHEMA__; Owner: __USER__
--

ALTER TABLE __SCHEMA__.users ADD CONSTRAINT uniq_username_users UNIQUE (username);


--
-- PostgreSQL database dump complete
--
