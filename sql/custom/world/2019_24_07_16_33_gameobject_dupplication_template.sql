CREATE TABLE IF NOT EXISTS `gameobject_dupplication_template` (
  `entry` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `account` int(10) unsigned NOT NULL,
  `referenceEntry` mediumint(8) unsigned NOT NULL,
  `referencePosZ` float NOT NULL,
  `referenceOrientation` float NOT NULL,
  `referenceSize` float NOT NULL,
  `author` varchar(50) NOT NULL DEFAULT 'null',
  `isPrivate` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM AUTO_INCREMENT=1662 DEFAULT CHARSET=utf8;

