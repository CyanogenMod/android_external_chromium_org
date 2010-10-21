PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY,value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','22');
INSERT INTO "meta" VALUES('last_compatible_version','21');
INSERT INTO "meta" VALUES('Default Search Provider ID','2');
INSERT INTO "meta" VALUES('Builtin Keyword Version','29');
CREATE TABLE keywords (id INTEGER PRIMARY KEY,short_name VARCHAR NOT NULL,keyword VARCHAR NOT NULL,favicon_url VARCHAR NOT NULL,url VARCHAR NOT NULL,show_in_default_list INTEGER,safe_for_autoreplace INTEGER,originating_url VARCHAR,date_created INTEGER DEFAULT 0,usage_count INTEGER DEFAULT 0,input_encodings VARCHAR,suggest_url VARCHAR,prepopulate_id INTEGER DEFAULT 0,autogenerate_keyword INTEGER DEFAULT 0);
INSERT INTO "keywords" VALUES(2,'Google','google.com','http://www.google.com/favicon.ico','{google:baseURL}search?{google:RLZ}{google:acceptedSuggestion}{google:originalQueryForSuggestion}sourceid=chrome&ie={inputEncoding}&q={searchTerms}',1,1,'',0,0,'UTF-8','{google:baseSuggestURL}search?client=chrome&hl={language}&q={searchTerms}',1,1);
INSERT INTO "keywords" VALUES(3,'Yahoo!','yahoo.com','http://search.yahoo.com/favicon.ico','http://search.yahoo.com/search?ei={inputEncoding}&fr=crmas&p={searchTerms}',1,1,'',0,0,'UTF-8','http://ff.search.yahoo.com/gossip?output=fxjson&command={searchTerms}',2,0);
INSERT INTO "keywords" VALUES(4,'Bing','bing.com','http://www.bing.com/s/wlflag.ico','http://www.bing.com/search?setmkt=en-US&q={searchTerms}',1,1,'',0,0,'UTF-8','http://api.bing.com/osjson.aspx?query={searchTerms}&language={language}',3,0);
INSERT INTO "keywords" VALUES(5,'Wikipedia (en)','en.wikipedia.org','','http://en.wikipedia.org/w/index.php?title=Special:Search&search={searchTerms}',1,0,'',1283287335,0,'','',0,0);
INSERT INTO "keywords" VALUES(6,'NYTimes','query.nytimes.com','','http://query.nytimes.com/gst/handler.html?query={searchTerms}&opensearch=1',1,0,'',1283287335,0,'','',0,0);
INSERT INTO "keywords" VALUES(7,'eBay','rover.ebay.com','','http://rover.ebay.com/rover/1/711-43047-14818-1/4?satitle={searchTerms}',1,0,'',1283287335,0,'','',0,0);
INSERT INTO "keywords" VALUES(8,'ff','ff','','http://ff{searchTerms}',0,0,'',1283287356,0,'','',0,0);
CREATE TABLE logins (origin_url VARCHAR NOT NULL, action_url VARCHAR, username_element VARCHAR, username_value VARCHAR, password_element VARCHAR, password_value BLOB, submit_element VARCHAR, signon_realm VARCHAR NOT NULL, ssl_valid INTEGER NOT NULL,preferred INTEGER NOT NULL, date_created INTEGER NOT NULL,blacklisted_by_user INTEGER NOT NULL, scheme INTEGER NOT NULL,UNIQUE (origin_url, username_element, username_value, password_element, submit_element, signon_realm));
CREATE TABLE ie7_logins (url_hash VARCHAR NOT NULL, password_value BLOB,date_created INTEGER NOT NULL,UNIQUE (url_hash));
CREATE TABLE web_app_icons (url LONGVARCHAR,width int,height int,image BLOB, UNIQUE (url, width, height));
CREATE TABLE web_apps (url LONGVARCHAR UNIQUE,has_all_images INTEGER NOT NULL);
CREATE TABLE autofill (name VARCHAR, value VARCHAR, value_lower VARCHAR, pair_id INTEGER PRIMARY KEY, count INTEGER DEFAULT 1);
CREATE TABLE autofill_dates ( pair_id INTEGER DEFAULT 0,date_created INTEGER DEFAULT 0);
CREATE TABLE autofill_profiles ( label VARCHAR, unique_id INTEGER PRIMARY KEY, first_name VARCHAR, middle_name VARCHAR, last_name VARCHAR, email VARCHAR, company_name VARCHAR, address_line_1 VARCHAR, address_line_2 VARCHAR, city VARCHAR, state VARCHAR, zipcode VARCHAR, country VARCHAR, phone VARCHAR, fax VARCHAR);
CREATE TABLE credit_cards ( label VARCHAR, unique_id INTEGER PRIMARY KEY, name_on_card VARCHAR, type VARCHAR,card_number VARCHAR, expiration_month INTEGER, expiration_year INTEGER, verification_code VARCHAR, billing_address VARCHAR, shipping_address VARCHAR, card_number_encrypted BLOB, verification_code_encrypted BLOB);
CREATE INDEX logins_signon ON logins (signon_realm);
CREATE INDEX ie7_logins_hash ON ie7_logins (url_hash);
CREATE INDEX web_apps_url_index ON web_apps (url);
CREATE INDEX autofill_name ON autofill (name);
CREATE INDEX autofill_name_value_lower ON autofill (name, value_lower);
CREATE INDEX autofill_dates_pair_id ON autofill_dates (pair_id);
CREATE INDEX autofill_profiles_label_index ON autofill_profiles (label);
CREATE INDEX credit_cards_label_index ON credit_cards (label);
COMMIT;
