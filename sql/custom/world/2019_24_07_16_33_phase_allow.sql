CREATE TABLE IF NOT EXISTS `phase_allow` (
  `phaseId` int(20) unsigned NOT NULL,
  `playerId` int(20) unsigned NOT NULL,
  `type` smallint(255) unsigned DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

