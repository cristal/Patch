SET NAMES utf8;

DELETE FROM `command` WHERE `name` LIKE '%jail%';
INSERT INTO `command` (name, security, help) VALUES
('jail info',       0, 'Syntax: .jail info\nShows your prison status.'),
('jail goto',       0, 'Syntax: .jail goto Faction (horde/ally)\nTeleports you to the jail of the specified faction.'),
('jail pinfo',      1, 'Syntax: .jail pinfo [Character]\nDisplays jail information of the specified character.'),
('jail arrest',     2, 'Syntax: .jail arrest Character Hours Reason\nArrests character for specified ammount hours and reason.'),
('jail release',    2, 'Syntax: .jail release Character\nReleases the character specified from jail.'),
('jail reset',      2, 'Syntax: .jail reset [Character]\nResets the specified characters arrest record as if nothing ever happened.'),
('jail reload',     2, 'Syntax: .jail reload\nReload jail configuration data.'),
('jail enable',     2, 'Syntax: .jail enable\nEnable jail system.'),
('jail disable',    2, 'Syntax: .jail disable\nDisable jail system.'),
('jail delete',     3, 'Syntax: .jail delete\nSame as reset but also clears any bans as well');
