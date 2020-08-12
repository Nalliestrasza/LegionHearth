CREATE TABLE IF NOT EXISTS `phase_custom_areaid` (
  `MapID` int(11) unsigned NOT NULL,
  `AreaID` int(11) unsigned NOT NULL,
  `MapName` text CHARACTER SET utf8,
  `AreaName` text CHARACTER SET utf8,
  PRIMARY KEY (`MapID`,`AreaID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
