# YAM [![Build Status](https://travis-ci.org/jens-maus/yam.svg?branch=master)](https://travis-ci.org/jens-maus/yam) [![PayPal Donate](https://img.shields.io/badge/paypal-donate-yellow.svg?style=flat)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=8L52PD9A9WS36)
YAM (short for **Y**et **A**nother **M**ailer) is a MIME-compliant open-source mail user agent (MUA) written for Amiga-based computer
systems (AmigaOS4, AmigaOS3, MorphOS, AROS). It supports POP3, SMTP, TLSv1/SSLv3 connection security, multiple users, multiple identities
, PGPv2/v5 encryption, unlimited hierarchical folders, filters, a configurable user interface based on the Magic User Interface (MUI)
framework and an ARexx interface and many other features which are common for MUAs today.

## Features
As the development of YAM goes back to 1995 it is one of the most feature-rich applications on the Amiga platform. Especially
the MUI-based user interface allowed to implement functionality which are commonly used in graphical mail user client on other platforms today. Furthermore, the focus on support and compliance to recent RFCs is one of the key goals of YAM.

* Straightforward installation and configuration, taking only a few minutes to set everything up.
* Runs "Out-of-the-Box" without any installation or Assign required for expert users.
* Easy operation using toolbar buttons, menus, keyboard and drag&drop functionality.
* Basic e-mail functions: Read, Write, Reply, Forward and Bounce mail
* Six standard folders: Incoming, Draft, Outgoing, Sent, Trash and Spam.
* Any number of user-definable folders, which can be compressed and/or encrypted via own passwords set.
* Hierarchical ordering of folders.
* Multiuser support. Optionally, address books and other configuration files can be shared.
* Searchable address book supporting groups and distribution lists.
* Full POP3 support allowing to check/download mails on startup, on demand or at regular time intervals.
* Configure an unlimited amount of POP3 accounts.
* Message download pre-selection: browse message headers and select only those mails you want to download.
* Write or Reply to your mails off-line and send them to the mail server using the built in SMTP support
* Extract sender information from message headers and create an address book entry with just a simple mouse click.
* Built-In support for MIME encoding/decoding for sending and receiving binary files.
* Interaction with web browsers: send mail from your browser, pass an URL to the browser.
* Handle message disposition notifications (MDN).
* Direct support for anonymous remailing (remailers) and mailing lists.
* PGP/MIME support: encrypt and/or sign outgoing messages, check signatures, decrypt messages. Direct support for PGP 2.6.x and PGP 5.
* Fast internal text editor with support for common mail styles (bold/italic/underline), glossary and with a spell checking interface.
* Comprehensive search capabilities. For example a full text search through all articles using a single query or a quick search to find/sort mails quickly in a folder.
* Automatic sorting of the post with an unlimited number of filters. Archiving or diversion of specified articles, automatic replies or the deletion of advertising are just a few of the possible applications of the filters.
* Event-sensitive starting of macros and comprehensive ARexx support.
* Freely customisable interface, thanks to MUI.
* Usable on either an own screen or on any public screen configured in MUI.
* Context-sensitive online help system, using help bubbles and online documentation.
* Context sensitive menus for direct operations on each mail.
* Localised to many languages.
* ... and much, much more.

## Downloads/Releases
All releases up to the most current ones can be downloaded from our [central releases management](https://github.com/jens-maus/yam/releases). In addition to the standard releases we are also offering regular development builds of the most recent code taken from this code repository. These nightly builds are available through http://nightly.yam.ch/ and are regularly updated based on the changes applied to this repository.

## Contributing
There are several ways how you can potentially contribute to this project. One important way to contribute to YAM is to actually *report bugs/issues* you might identify. In addition you can also bring up feature/enhancements requests by using our [central issue tracker](https://github.com/jens-maus/yam/issues). Another way is to *help us translating* the user interface to a wide range of different languages by actually contributing your translations at the [transifex project pages](https://www.transifex.com/ato/yam/) we are maintaining to manage all translations.  

## Development
Contributing your own code/modifications to YAM is quite straight forward since you can use the nice resources of GitHub and submit your changes in terms of [pull requests](https://github.com/jens-maus/yam/pulls). If you, however, feel you might be better suited by joining our development team directly and you would like to directly submit your changes to this code repository we could easily provide you direct write access.

## License & Copyright
YAM is distributed and license under the GNU General Public License Version 2. See [COPYING](COPYING) for more detailed information.
