SET NAMES utf8;

DROP TABLE IF EXISTS `jail`;
CREATE TABLE `jail` (
  `guid` int(11) unsigned NOT NULL COMMENT 'GUID of Jail Inmate',
  `char` varchar(13) NOT NULL COMMENT 'Character Name of Jail Inmate',
  `release` int(11) unsigned NOT NULL COMMENT 'Release Time',
  `reason` varchar(255) NOT NULL COMMENT 'Reason for Arrest',
  `times` int(11) unsigned NOT NULL COMMENT 'Number of Times Jailed',
  `gmacc` int(11) unsigned NOT NULL COMMENT 'Arresting GM Account',
  `gmchar` varchar(13) NOT NULL COMMENT 'Arresting GM Character',
  `lasttime` int(11) unsigned NOT NULL COMMENT 'Last Time Jailed',
  `duration` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Duration of Jail Time',
  `btimes` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Number of Times Banned by Jail',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Jail System Inmate Registry Table.';

DROP TABLE IF EXISTS `jail_conf`;
CREATE TABLE `jail_conf` (
  `max_jails` int(11) unsigned NOT NULL default '3' COMMENT 'Number times a player can be jailed before they can be banned or deleted.',
  `max_duration` int(11) unsigned NOT NULL default '720' COMMENT 'Max number of hours for jail sentence (720 hours is 30 days).',
  `min_reason` int(11) unsigned NOT NULL default '40' COMMENT 'Minimum character number of text needed for reason.',
  `warn_player` tinyint(4) NOT NULL default '1' COMMENT 'Warn player at login when they are close to their jail limit before they are banned or deleted? (0 for false and 1 for true).',
  `ally_x` float NOT NULL default '-8673.43' COMMENT 'Alliance Jail X Coordinate',
  `ally_y` float NOT NULL default '631.795' COMMENT 'Alliance Jail Y Coordinate',
  `ally_z` float NOT NULL default '96.9406' COMMENT 'Alliance Jail Z Coordinate',
  `ally_o` float NOT NULL default '2.1785' COMMENT 'Alliance Jail Orientation',
  `ally_m` int(11) unsigned NOT NULL default '0' COMMENT 'Alliance Jail Map (0 for Alliance)',
  `horde_x` float NOT NULL default '2179.85' COMMENT 'Horde Jail X Coordinate',
  `horde_y` float NOT NULL default '-4763.96' COMMENT 'Horde Jail Y Coordinate',
  `horde_z` float NOT NULL default '54.911' COMMENT 'Horde Jail Z Coordinate',
  `horde_o` float NOT NULL default '4.44216' COMMENT 'Horde Jail Orientation',
  `horde_m` int(11) unsigned NOT NULL default '1' COMMENT 'Horde Jail Map (1 for Horde)',
  `del_char` tinyint(4) NOT NULL default '0' COMMENT 'Delete character when they have reached max times in jail count? (0 for no or 1 for yes).',
  `ban_acc` tinyint(4) NOT NULL default '1' COMMENT 'Ban account when they have reached max times in jail count? (0 for no or 1 for yes).',
  `ban_duration` int(11) unsigned NOT NULL default '168' COMMENT 'Number of hours player ban by jail lasts (168 hours is 7 days).',
  `radius` int(11) unsigned NOT NULL default '10' COMMENT 'Yards a player can move while they are in jail (0 denotes player cant move at all).',
  `enabled` tinyint(4) NOT NULL default '0' COMMENT 'Enable or Disable jail system (0 for disable or 1 for enable).',
  `gm_acc` int(11) unsigned NOT NULL default '1' COMMENT 'Account used for banning.',
  `gm_char` varchar(13) NOT NULL default 'Admin' COMMENT 'Character used for banning.',
  `amnesty` int(11) unsigned NOT NULL default '3' COMMENT 'Number of month(s) jail record entry expires.',
  PRIMARY KEY  (`max_jails`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Jail System Configuration Table.';

INSERT INTO `jail_conf` (`max_jails`,`max_duration`,`min_reason`,`warn_player`,`ally_x`,`ally_y`,`ally_z`,`ally_o`,`ally_m`,
`horde_x`,`horde_y`,`horde_z`,`horde_o`,`horde_m`,`del_char`,`ban_acc`,`ban_duration`,`radius`,`enabled`) VALUES
(3,672,30,1,31.7282,135.794,-40.0508,4.73516,35,2179.85,-4763.96,54.911,4.44216,1,0,1,168,15,1);
