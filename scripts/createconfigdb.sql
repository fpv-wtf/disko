/************************************************************/
/*		                                            */
/* Create table plugins                                     */
/*                                                          */
/************************************************************/
CREATE TABLE IF NOT EXISTS Plugins 
(
	ID INTEGER NOT NULL PRIMARY KEY,
	PluginName VARCHAR NOT NULL UNIQUE,
	PluginTitle VARCHAR NOT NULL,
	PluginDescription VARCHAR,
	Filename VARCHAR NOT NULL,
	PluginPath VARCHAR NOT NULL,
	Active CHAR(1) NOT NULL,
	Icon VARCHAR,
	SelectedIcon VARCHAR,
	SmallIcon VARCHAR,
	SmallSelectedIcon VARCHAR,
	PluginTypeID INTEGER CONSTRAINT FK_PluginTypeID REFERENCES PluginTypes(ID),
	CategoryID INTEGER CONSTRAINT FK_CategoryID REFERENCES Category(ID),
	Orderpos INTEGER,
	Version VARCHAR DEFAULT "1.0"
);

/************************************************************/
/*		                                            */
/* Create table plugins                                     */
/*                                                          */
/************************************************************/
CREATE TABLE IF NOT EXISTS Category
(
	ID INTEGER NOT NULL PRIMARY KEY,
	CategoryName VARCHAR NOT NULL UNIQUE
);

/************************************************************/
/*		                                            */
/* create table PluginProperties                            */
/*		                                            */
/************************************************************/
CREATE TABLE IF NOT EXISTS PluginProperties 
(
	Parameter VARCHAR NOT NULL,
	Value VARCHAR NOT NULL,
	TYPE VARCHAR CHECK (TYPE IN ('string','integer','float','set','enum')) DEFAULT 'string',
	MAX INTEGER,
	MIN INTEGER,
	VALLIST VARCHAR,
	SEPARATOR CHAR(1),
	PluginID INTEGER,
	PRIMARY KEY (Parameter, Value, PluginID)
);

/************************************************************/
/*		                                            */
/* Create table PluginTypes                                 */
/*		                                            */
/************************************************************/
CREATE TABLE IF NOT EXISTS PluginTypes 
(
	ID INTEGER NOT NULL PRIMARY KEY,
	PluginTypeName VARCHAR UNIQUE
);

/************************************************************/
/*                                                          */
/* Create table ImportProperties                            */
/*                                                          */
/************************************************************/
CREATE TABLE IF NOT EXISTS ImportProperties 
(
	ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	PluginID INTEGER NOT NULL UNIQUE,
	onStartUp CHAR(1),
	Time INTEGER,
	Interval INTEGER
);

/************************************************************/
/*		                                            */
/* Create table ImportSource                                */
/*		                                            */
/************************************************************/
CREATE TABLE IF NOT EXISTS ImportSource 
(
	ID INTEGER NOT NULL PRIMARY KEY ASC AUTOINCREMENT,
	PluginID INTEGER NOT NULL,
	Name VARCHAR,
	Source VARCHAR,
	LifeTime INTEGER,

	UNIQUE (PluginID, Source)
);


/************************************************************/
/*		                                            */
/* This trigger creates the foreign key for plugins table.  */
/* If plugins will be inserted the trigger checks if the    */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fki_plugins_plugintypes_id BEFORE INSERT ON Plugins
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM PluginTypes WHERE ID = NEW.PluginTypeID) IS NULL)
     THEN RAISE(ABORT, 'insert on table "Plugins" violates foreign key. [PluginType doesnt exist]')
  END;
END;

/************************************************************/
/*		                                            */
/* This trigger creates the foreign key for plugins table.  */
/* If a plugin will be updated the trigger checks if the    */
/* plugintype value is valid.                               */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fku_plugins_plugintypes_id BEFORE UPDATE ON Plugins
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM PluginTypes WHERE ID = NEW.PluginTypeID) IS NULL)
     THEN RAISE(ABORT, 'update on table "Plugins" violates foreign key. [PluginType doesnt exist]')
  END;
END;


/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fkd_plugins_plugintypes_id BEFORE DELETE ON Plugintypes
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT PluginTypeID FROM Plugins WHERE PluginTypeID = ID))
     THEN RAISE(ABORT, 'delete of PluginType violates foreign key. [PluginType used by Plugins]')
  END;
END;


/************************************************************/
/*		                                            */
/* This trigger delete automatically the PluginProperties   */
/* entries of a plugin if a plugin should be deleted. 	    */
/* Its more practically because of you only need to delete  */
/* the plugins.	All other item of the plugin will be 	    */
/* removed automatically.				    */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fkd_plugins BEFORE DELETE ON Plugins
FOR EACH ROW BEGIN 
    DELETE FROM PluginProperties WHERE PluginID = OLD.ID;
    DELETE FROM ImportProperties WHERE PluginID = OLD.ID;
END;


/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fki_pluginproperties_plugins_id BEFORE INSERT ON PluginProperties
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM Plugins WHERE ID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'insert on table "PluginProperties" violates foreign key. [PluginID doesnt exist]')
  END;
END;


/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fku_pluginproperties_plugins_id BEFORE UPDATE ON PluginProperties
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM Plugins WHERE ID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'update on table "PluginProperties" violates foreign key. [PluginID doesnt exist]')
  END;
END;

/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fki_importproperties_plugins_id BEFORE INSERT ON ImportProperties
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM Plugins WHERE ID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'insert on table "ImportProperties" violates foreign key. [PluginID doesnt exist]')
  END;
END;


/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fku_importproperties_plugins_id BEFORE UPDATE ON ImportProperties
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT ID FROM Plugins WHERE ID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'update on table "ImportProperties" violates foreign key. [PluginID doesnt exist]')
  END;
END;

/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fkd_importproperties BEFORE DELETE ON ImportProperties
FOR EACH ROW BEGIN 
    DELETE FROM ImportSource WHERE PluginID = OLD.PluginID;
END;


/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fki_importsource_importproperties_pluginid BEFORE INSERT ON ImportSource
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT PluginID FROM ImportProperties WHERE PluginID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'insert on table "ImportSource" violates foreign key. [PluginID in ImportProperties doesnt exist]')
  END;
END;

/************************************************************/
/*		                                            */
/*		                                            */
/************************************************************/
CREATE TRIGGER IF NOT EXISTS fku_importsource_importproperties_pluginid BEFORE UPDATE ON ImportSource
FOR EACH ROW BEGIN 
  SELECT CASE
     WHEN ((SELECT PluginID FROM ImportProperties WHERE PluginID = NEW.PluginID) IS NULL)
     THEN RAISE(ABORT, 'update on table "ImportSource" violates foreign key. [PluginID in ImportProperties doesnt exist]')
  END;
END;

/************************************************************/
/*		                                                    */
/*	Insert required data                                    */
/*		                                                    */
/************************************************************/
INSERT OR IGNORE INTO PluginTypes (PluginTypeName) VALUES("OSD_PLUGIN");
INSERT OR IGNORE INTO PluginTypes (PluginTypeName) VALUES("BACKEND_PLUGIN");
INSERT OR IGNORE INTO PluginTypes (PluginTypeName) VALUES("IMPORT_PLUGIN");
INSERT OR IGNORE INTO PluginTypes (PluginTypeName) VALUES("CENTRAL_PLUGIN");
