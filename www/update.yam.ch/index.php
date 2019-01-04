<?php
/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2019 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

***************************************************************************/
 
/*
 * PHP script to manage the update requests to the http://update.yam.ch/
 * web server for checking/notifying clients that certain components are
 * available in newer versions.
 *
 * Requests are commonly formed in the following form and an example
 * request might look like:
 *
 * http://update.yam.ch/?ver=YAM%202%2E6%2Ddev%20%5BOS4%2FPPC%5D&builddate=08%2E02%2E2008&lang=deutsch%206%2E0&exec=52%2E21&lib0=codesets-6%2E5&lib1=amissl-3%2E7&lib2=xpk-5%2E2&lib3=openurl-7%2E2&mui=20%2E2282&mcc0=thebar-26%2E1&mcc1=texteditor-15%2E26&mcc2=betterstring-11%2E14&mcc3=nlist-20%2E120&mcc4=nlistview-19%2E75&mcc5=nfloattext-19%2E56&mcc6=nlisttree-18%2E27
 *
 */

// path to the various update directories
$NIGHTLYDIR="updates/nightly";
$STABLEDIR="updates/stable";
$CONTRIBDIR="updates/contrib";

// path to the logfile where we log our visitors/updaters
$LOGFILE="log/updatecheck.log";

// retrieve the data a user might specify on the request URL by using
// an URL similar to: http://update.yam.ch/?ver=2.5&buildid=20060505
// Please note that such URLs have to be properly encoded in the common
// URL encoding format.
$userVERSTR=trim(strtolower($_GET['ver']));              // version (YAM 2.5-dev [OS4/PPC])
$userBUILDID=trim(strtolower($_GET['buildid']), " \t-"); // buildid (20060505)
$userBUILDDATE=trim(strtolower($_GET['builddate']));     // builddate (05.05.2006)
$userLANG=trim(strtolower($_GET['lang']));               // catalog language and version (deutsch 5.5)
$userEXEC=trim(strtolower($_GET['exec']));               // users' exec version (51.16)
$userMUI=trim(strtolower($_GET['mui']));                 // users' MUI version (20.1551)
$debug=$_GET['debug'];                                   // if "1" then be more verbose

// before we do anything serious, we do a check if all necessary options
// were specified and if not we simply generate a fake webpage redirecting to the main
// webpage of www.yam.ch
if($userVERSTR == "" || $userBUILDDATE == "")
{
   // redirect the user to the download description page
   print "<head>";
   print "<meta http-equiv='refresh' content='1; URL=http://yam.ch/downloads'>";
   print "</head>";

   // exit here and do not continue processing
   exit;
}

// log the visitor/updater in our logfile first
$ipaddress=$_SERVER['REMOTE_ADDR'];
$referrer=$_SERVER['HTTP_REFERER'];
$datetime=date('Y-m-d H:i:s');
$useragent = $_SERVER['HTTP_USER_AGENT'];
$remotehost = @getHostByAddr($ipaddress);

// open logfile in append mode and output some information for
// statistical purposes. Here we take care not to catch any utterly confidental
// information about the client system. Only the bare minimum like which
// version of YAM and its components and where the request came from.
if($fh = fopen($LOGFILE, 'a+'))
{
   $logline = "$datetime: $remotehost [$ipaddress]: $userVERSTR ($userBUILDDATE) - $userLANG - exec $userEXEC - MUI $userMUI\n";

   fwrite($fh, $logline);

   fclose($fh);
}

// now open a temporary file to which we output all the stuff here
// and afterwards we send it over to the users
$outfile = tempnam("/tmp", "yam");
if($out = fopen($outfile, 'w'))
{
  // print out some initial output (version of php script and timestamp
  fprintf($out, "<updatecheck>\n");
  fprintf($out, "\$VER: updatecheck 1.3 (06.03.2018)\n");
  fprintf($out, "TIME: %s\n", $datetime);
  
  // what we have to do now is: check each single component of YAM for
  // an eventually existing update and report it to the user
  
  // 1. Check if the user uses a nightly build or not
  if(preg_match("/(dev|nightly)/i", $userVERSTR) && (preg_match("^[0-9]+$", $userBUILDID) || $userBUILDID == ""))
    $nightly=true;
  else
    $nightly=false;
  
  // 2. Check for "YAM" update itself
  list($prog, $ver, $system) = explode(" ", trim($userVERSTR), 3);
  if($nightly == true)
    $updateDir = $NIGHTLYDIR;
  else
    $updateDir = $STABLEDIR;
  
  // extract the real userversion, userrevision and
  // patchlevel numbers
  list($userVER, $userREV, $userPL) = preg_split("(\.|p|-)", trim($ver), 3);
  $userVER = intval($userVER);
  $userREV = intval($userREV);
  $userPL = intval($userPL);
  
  // analyze the target specification (if present)
  if(preg_match("/os4\/ppc/i", $userVERSTR))
    $userTARGET = "ppc-amigaos";
  elseif(preg_match("/mos\/ppc/i", $userVERSTR))
    $userTARGET = "ppc-morphos";
  elseif(preg_match("/aros\/x86_64/i", $userVERSTR))
    $userTARGET = "x86_64-aros";
  elseif(preg_match("/aros\/x86/i", $userVERSTR))
    $userTARGET = "i386-aros";
  elseif(preg_match("/aros\/ppc/i", $userVERSTR))
    $userTARGET = "ppc-aros";
  else
    $userTARGET = "m68k-amigaos";

  // now we should know all major stuff like the version/revision and
  // if the user version is a nightly build or not. So what we do now is, that
  // we go and check the 'LATEST' file of the stable tree first and in case the
  // user is using a nightly build we check against that latest stable version
  // as well.
  $fh=fopen($STABLEDIR . "/LATEST", 'r');
  if($fh)
  {
    while(!feof($fh))
    {
      $line = fgets($fh);
      list($stableTarget, $stableYAM) = explode(": ", trim($line), 2);
  
      // check if we found the target 
      if($userTARGET == $stableTarget)
      {
        // split the stableYAM by version/revision
        list($stableVER, $stableREV, $stablePL) = preg_split("(\.|p|-)", trim($stableYAM), 3);
        $stableVER = intval($stableVER);
        $stableREV = intval($stableREV);
        $stablePL = intval($stablePL);

        // now break out
        break;
      }
    }
  
    fclose($fh);
  }
  
  #printf("%d %d %d : %d %d %d\n", $userVER, $userREV, $userPL, $stableVER, $stableREV, $stablePL);
  
  // now we should know what the very latest stable version is and all the necessary USER
  // version information as well. Here we check first for plausible values first or
  // something went definitly wrong.
  if($userVER >= 2 && $userREV >= 4 && $userPL >= 0 &&
     $stableVER >= 2 && $stableREV >= 4 && $stablePL >= 0)
  {
     // check if the user version is a nightly build and if so we check
     // the stable version against the user version and in case the stable version is
     // higher than the users' one we suggest the user to update to the very latest
     // stable one instead of cross migrating to the next nightly's of the next version
     if($nightly == true && ($stableVER < $userVER || 
                             ($stableVER == $userVER && $stableREV < $userREV)))
     {
        // now we can update the nightly build of the user because the versions
        // match each other.
        if(file_exists($updateDir . "/" . $userVER . "." . $userREV) &&
           $fh = fopen($updateDir . "/" . $userVER . "." . $userREV, 'r'))
        {
           $updateFound = false;
           $changelog = false;
  
           while(!feof($fh))
           {
              $line = fgets($fh);
              list($tag, $value1, $value2, $value3) = explode(" ", trim($line), 4);
  
              if($changelog == true)
                 fputs($out, $line);
              elseif($tag == "VERSION:")
                 $updateVER = $value1;
              elseif($tag == "BUILDID:")
                 $updateBUILDID = $value1;
              elseif($tag == "BUILDDATE:")
              {
                 $updateBUILDDATE = $value1;
  
                 // we now check if this update file really carries newer stuff
                 // or if we simply have to break out here.
                 if($userBUILDDATE != "" && strtotime($userBUILDDATE) >= strtotime($updateBUILDDATE))
                    break;
              }
              elseif($tag == "URL:" && ($value1 == $userTARGET || $value1 == "*"))
              {
                 // output the initial [UPDATE] header only once
                 if($updateFound == false)
                 {
                    fprintf($out, "<component>\n");
                    fprintf($out, "NAME: YAM\n");
                    fprintf($out, "TARGET: $userTARGET\n");
  
                    if($updateBUILDID != "")
                       fprintf($out, "RECENT: %s-%s\n", $updateVER, $updateBUILDID);
                    else
                       fprintf($out, "RECENT: %s\n", $updateVER);
  
                    if($userBUILDID != "")
                       fprintf($out, "INSTALLED: %s-%s\n", $ver, $userBUILDID);
                    else
                       fprintf($out, "INSTALLED: %s\n", $ver);
  
                    $updateFound = true;
                 }
                 
                 // now we output the "URL:" line
                 fprintf($out, "URL: %s\n", trim($value2 . " " . $value3));
              }
              elseif($tag == "CHANGES:")
              {
                 fprintf($out, "<changelog>\n");
                 $changelog = true;
              }
           }
  
           if($changelog == true)
              fprintf($out, "</changelog>\n");
  
           if($updateFound == true)
              fprintf($out, "</component>\n");
   
           fclose($fh);
        }
     }
     else
     {
        // now we have to check if we should suggest the user to update to the very
        // latest stable version available instead
        if(file_exists($STABLEDIR . "/" . $stableYAM) &&
           $fh = fopen($STABLEDIR . "/" . $stableYAM, 'r'))
        {
           $updateFound = false;
           $changelog = false;
  
           while(!feof($fh))
           {
              $line = fgets($fh);
              list($tag, $value1, $value2, $value3) = explode(" ", trim($line), 4);
  
              if($changelog == true)
                 fputs($out, $line);
              elseif($tag == "VERSION:")
              {
                 $updateVER = $value1;
  
                 // check if the version is really newer or equal to the user one
                 list($updateVERSION, $updateREVISION, $updatePL) = preg_split("(\.|p|-)", trim($updateVER), 3);
                 $updateVERSION = intval($updateVERSION);
                 $updateREVISION = intval($updateREVISION);
                 $updatePL = intval($updatePL);

                 if($userVER > $updateVERSION || 
                    ($userVER == $updateVERSION && $userREV > $updateREVISION) ||
                    ($userVER == $updateVERSION && $userREV == $updateREVISION && $userPL > $updatePL))
                 {
                    break;
                 }
              }
              elseif($tag == "BUILDID:")
                 $updateBUILDID = $value1;
              elseif($tag == "BUILDDATE:")
              {
                 $updateBUILDDATE = $value1;
  
                 // we now check if this update file really carries newer stuff
                 // or if we simply have to break out here.
                 if($userBUILDDATE != "" && strtotime($userBUILDDATE) >= strtotime($updateBUILDDATE))
                    break;
              }
              elseif($tag == "URL:" && ($value1 == $userTARGET || $value1 == "*"))
              {
                 // output the initial [UPDATE] header only once
                 if($updateFound == false)
                 {
                    fprintf($out, "<component>\n");
                    fprintf($out, "NAME: YAM\n");
                    fprintf($out, "TARGET: $userTARGET\n");
  
                    if($updateBUILDID != "")
                       fprintf($out, "RECENT: %s-%s\n", $updateVER, $updateBUILDID);
                    else
                       fprintf($out, "RECENT: %s\n", $updateVER);
  
                    if($userBUILDID != "")
                       fprintf($out, "INSTALLED: %s-%s\n", $ver, $userBUILDID);
                    else
                       fprintf($out, "INSTALLED: %s\n", $ver);
  
                    $updateFound = true;
                 }
                 
                 // now we output the "URL:" line
                 fprintf($out, "URL: %s\n", trim($value2 . " " . $value3));
              }
              elseif($tag == "CHANGES:")
              {
                 fprintf($out, "<changelog>\n");
                 $changelog = true;
              }
           }
  
           if($changelog == true)
              fprintf($out, "</changelog>\n");
  
           if($updateFound == true)
              fprintf($out, "</component>\n");
   
           fclose($fh);
        }
     }
  }
  
  /////////////////////
  // now that we have checked/analyzed the update status of YAM itself, we can now
  // analyze the update status of the contributions including the version of the
  // used catalog translations (locale).
  
  // first we check all 'mcc' variables and then the 'lib' variables
  // of the update request
  for($i=0; $i < 2; $i++)
  {
     // first check for mcc's then for libraries
     if($i == 0)
       $var = "mcc";
     elseif($i == 1)
       $var = "lib";
  
     $j=0;
     while(($lib=trim(strtolower($_GET["$var$j"]))) != "")
     {
       // extract the name and version
       list($name, $verstr) = preg_split("(-)", $lib, 2);
  
       // open the version file
       if(file_exists($CONTRIBDIR . "/" . $name . ".$var") &&
          $fh = fopen($CONTRIBDIR . "/" . $name . ".$var", 'r'))
       {
         // extract the correct version/revision parts of verstr
         list($version, $revision) = preg_split("(\.)", $verstr, 2);
         $version = intval($version);
         $revision = intval($revision);
         $changelog = false;
     
         while(!feof($fh))
         {
            $line = fgets($fh);
            list($tag, $value1, $value2, $value3) = explode(" ", trim($line), 4);
     
            if($changelog == true)
               fputs($out, $line);
            elseif($tag == "NAME:")
               $updateNAME = trim($value1);
            elseif($tag == "VERSION:")
            {
               $updateVER = trim($value1);
     
               // split the VERSION string into version and revision
               list($libVersion, $libRevision) = preg_split("(\.)", $updateVER, 2);
               $libVersion = intval($libVersion);
               $libRevision = intval($libRevision);
     
               // we now check if this update file really carries newer stuff
               // or if we simply have to break out here.
               if($updateVER == "" || 
                  $libVersion < $version ||
                  ($libVersion == $version && $libRevision <= $revision))
               {
                  // no update required
                  $updateFound = false;
                  break;
               }
     
               // issue a new component header
               fprintf($out, "<component>\n");
               fprintf($out, "NAME: $updateNAME\n");
               fprintf($out, "TARGET: $userTARGET\n");
               fprintf($out, "RECENT: $libVersion.$libRevision\n");
               fprintf($out, "INSTALLED: $version.$revision\n");
     
               // this component requires an update lets notify the user
               $updateFound = true;
            }
            elseif($tag == "BUILDDATE:")
               fprintf($out, "BUILDDATE: $value1\n");
            elseif($tag == "URL:" && ($value1 == $userTARGET || $value1 == "*"))
               fprintf($out, "URL: %s\n", trim($value2 . " " . $value3));
            elseif($tag == "CHANGES:")
            {
               fprintf($out, "<changelog>\n");
               $changelog = true;
            }
         }
     
         if($changelog == true)
            fprintf($out, "</changelog>\n");
     
         if($updateFound == true)
            fprintf($out, "</component>\n");
     
         fclose($fh);
       }
     
       $j++;
     }
  }
  
  // now close the update information with an </updatecheck> tag.
  fprintf($out, "</updatecheck>\n");
  
  fclose($out);
}

// now we should have prepared the temporary file. let's get its
// file size
$outsize = filesize($outfile);

// as YAM 2.5's update check mechanism seems to be broken we
// need to manually add a Content-Length: header with the number 
// of bytes the request will sent.
//if(strncasecmp($useragent, "YAM/2.5", 7) == 0)
  header("Content-Length: " . $outsize);

// now we open the file again and output it step by step
if($out = fopen($outfile, 'r'))
{
  printf("%s", fread($out, $outsize));
  fclose($out);
}

unlink($outfile);

?>
