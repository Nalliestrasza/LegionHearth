CREATE TABLE IF NOT EXISTS `hwid_bans` (
  `accountID` int(10) unsigned NOT NULL DEFAULT '0',
  `physicalDriveID` int(10) unsigned NOT NULL DEFAULT '0',
  `cpuID` int(11) unsigned NOT NULL DEFAULT '0',
  `volumeInformation` int(11) unsigned NOT NULL DEFAULT '0',
  `ipAddress` text,
  `banned` tinyint(4) NOT NULL DEFAULT '0',
  `isVM` tinyint(4) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


