CREATE TABLE IF NOT EXISTS `ticket` (
  `ticketId` int(10) unsigned DEFAULT NULL,
  `ticketContents` varchar(255) DEFAULT NULL,
  `ticketOwner` varchar(255) DEFAULT NULL,
  `ticketOwnerAccId` int(10) unsigned DEFAULT NULL,
  `ticketOwnerGuid` int(10) unsigned DEFAULT NULL,
  `ticketStatus` tinyint(3) unsigned DEFAULT '0',
  `ticketSolutions` varchar(255) DEFAULT NULL,
  `ticketClosedBy` varchar(255) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;