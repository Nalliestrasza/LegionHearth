CREATE TABLE IF NOT EXISTS `gameobject_tele` (
  `entry` int(11) unsigned NOT NULL DEFAULT '0',
  `pos_x` float NOT NULL DEFAULT '0',
  `pos_y` float NOT NULL DEFAULT '0',
  `pos_z` float NOT NULL DEFAULT '0',
  `mapid` smallint(10) unsigned NOT NULL,
  `orientation` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;