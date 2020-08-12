CREATE TABLE IF NOT EXISTS `gameobject_dupplication_doodads` (
  `guid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `entry` int(10) unsigned NOT NULL,
  `objectID` int(10) unsigned NOT NULL,
  `diffX` float NOT NULL,
  `diffY` float NOT NULL,
  `diffZ` float NOT NULL,
  `diffO` float NOT NULL,
  `size` float NOT NULL,
  `rotationX` float NOT NULL,
  `rotationY` float NOT NULL,
  `rotationZ` float NOT NULL,
  `rotationW` float NOT NULL,
  `distance` float NOT NULL,
  `angle` float NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM AUTO_INCREMENT=216615 DEFAULT CHARSET=utf8;