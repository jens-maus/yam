/* arexx script to read Palm Address.dat (desktop) file and convert it to
   Yam address book format.

   filename: pda2yam.rexx
   version:   1.0
   author:    Brian Savage
              Mississauga, Ontario, Canada

   email:     b.savage@rogers.com
   website:   http://members.rogers.com/b.savage

   date:      September 2002

*/

options results

/* initialize some variables */

TRUE=1
FALSE=0

delimiter='\'
CR='0d'x
LF='0a'x

catlongname.0 = 'Unfiled'

phonetype.0 = 'W'
phonetype.1 = 'H'
phonetype.2 = 'F'
phonetype.3 = 'O'
phonetype.4 = 'E'
phonetype.5 = 'M'
phonetype.6 = 'P'
phonetype.7 = 'C'

/* palm fields */

LASTNAME = '1'
FIRSTNAME = '2'
TITLE = '3'
COMPANY = '4'
PHONE1ID = '5'
PHONE2ID = '6'
PHONE3ID = '7'
PHONE4ID = '8'
PHONE5ID = '9'
PHONE1 = '10'
PHONE2 = '11'
PHONE3 = '12'
PHONE4 = '13'
PHONE5 = '14'
ADDRESSTEXT = '15'
CITY = '16'
STATE = '17'
ZIP = '18'
COUNTRY = '19'
NOTE = '20'
CATEGORY = '21'
CUSTOM1 = '22'
CUSTOM2 = '23'
CUSTOM3 = '24'
CUSTOM4 = '25'

/* some of my own custom fields */

ALIAS= '26'

/* script looks for default values for source & destination files in same directory */

sourcefile='address.dat'
destfile='palm.addressbook'

/* parse the arguments */

parse arg a.1 a.2 a.3 a.4

do i=1 to 3
  j=i+1
  if upper(a.i)='FROM' Then
     if (a.j~='') then sourcefile = strip(a.j)
end

do i=1 to 3
  j=i+1
  if upper(a.i)='TO' Then
     if (a.j~='') then destfile = strip(a.j)
end

/* check if source file exists */

if ~exists(sourcefile) then
    do
       say "Can't find sourcefile; exiting script"
       exit
    end

/* check if destination file exists and if it does check if it an be over-written */

if exists(destfile) then
    do
       say 'Destination file: "'||destfile||'" exists. Overwrite? y/n'
       pull yesno
       if yesno='N' then
         do
            say 'Exiting script'
            exit
         end
    end


/* open palm desktop address book */

if ~open('adat',sourcefile,'Read') then
    do
      say "Can't open sourcefile; exiting script"
      exit
    end

/* check version tag; if first 4 bytes do not match address.dat format then exit script */

version_tag = readch('adat',4)

if version_tag ~='00014241'x then
    do
      say 'Source file is not a PalmOS Address.dat file; exiting script'
      exit
    end

/* read file name */

name_len = readch('adat',1)

fname=readch('adat',c2d(name_len))

/* read custom names */

table_string = GetCString()

/* next free category id */

nextfreecat = c2d(reverse(readch('adat',4)))

/* category count */

catcount = c2d(reverse(readch('adat',4)))

/* load categories */

do i=1 to catcount

    catindex.i = c2d(reverse(readch('adat',4)))
    catid.i = c2d(reverse(readch('adat',4)))
    catdirtyflag.i = c2d(reverse(readch('adat',4)))

    /* find category long names */

    catlongname.i = GetCString()

    /* find category short names */

    catshortname.i = GetCString()

end

/* load resource id */

    resourceid = c2d(reverse(readch('adat',4)))

/* load fields per row */

   fieldsperrow = c2d(reverse(readch('adat',4)))

/* record id */

   recid = c2d(reverse(readch('adat',4)))

/* record status */

   recstat =  c2d(reverse(readch('adat',4)))

/* placement position */

   placepos = c2d(reverse(readch('adat',4)))

/* field count (2) */

   fieldcount = c2d(reverse(readch('adat',2)))

/* field entries (2)    */

   fieldentries = readch('adat',60)

/* number of entries    */

   numrecords = c2d(reverse(readch('adat',4)))/30


/* let's read the addresses */

do j=1 to numrecords

/* last name */

    jump = seek('adat',32,'Current') /* skip 32 bytes of data that we don't need */

    pda.LASTNAME.j = GetCString()

/* first name */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.FIRSTNAME.j = GetCString()

/* title text */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.TITLE.j = GetCString()

/* company name */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.COMPANY.j = GetCString()

/* phone 1 text */

    jump = seek('adat',4,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE1ID.j = c2d(reverse(readch('adat',4)))
    jump = seek('adat',8,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE1.j = GetCString()

/* phone 2 text */

    jump = seek('adat',4,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE2ID.j = c2d(reverse(readch('adat',4)))
    jump = seek('adat',8,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE2.j = GetCString()

/* phone 3 text */

    jump = seek('adat',4,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE3ID.j = c2d(reverse(readch('adat',4)))
    jump = seek('adat',8,'Current') /* skip 16 bytes of data that we don't need */

    pda.PHONE3.j = GetCString()

/* phone 4 text */

    jump = seek('adat',4,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE4ID.j = c2d(reverse(readch('adat',4)))
    jump = seek('adat',8,'Current') /* skip 16 bytes of data that we don't need */

    pda.PHONE4.j = GetCString()

/* phone 5 text */

    jump = seek('adat',4,'Current') /* skip 16 bytes of data that we don't need */
    pda.PHONE5ID.j = c2d(reverse(readch('adat',4)))
    jump = seek('adat',8,'Current') /* skip 16 bytes of data that we don't need */

    pda.PHONE5.j = GetCString()

/* address */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.ADDRESSTEXT.j = GetCString()

/* city */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.CITY.j = GetCString()

/* state */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.STATE.j = GetCString()

/* zip */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.ZIP.j = GetCString()

/* country */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.COUNTRY.j = GetCString()

/* note */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.NOTE.j = GetCString()

/* category */

    jump = seek('adat',12,'Current') /* skip 12 bytes of data that we don't need */

    pda.CATEGORY.j = c2d(reverse(readch('adat',4)))

/* custom 1 */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.CUSTOM1.j = GetCString()

/* custom 2 */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.CUSTOM2.j = GetCString()

/* custom 3 */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.CUSTOM3.j = GetCString()

/* custom 4 */

    jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.CUSTOM4.j = GetCString()

jump = seek('adat',8,'Current') /* skip 8 bytes of data that we don't need */

    pda.ALIAS.j = pda.FIRSTNAME.j||' '||pda.LASTNAME.j /* this is my own custom field that I have added here */
end

close('adat')

/*==========================================================================================*/

/* loading of address.dat data file is now complete and is stored in an Arexx stem variable
   of the form pda.x.y where 'x' is the number of fields for each record (26; listed near top of file
   and 'y' is the number of records.

   The script can be branched at this point to produce other modules (ie CSV file, Cotnact data file etc)
*/

/*===========================================================================================*/

say numrecords 'records read from file' fname

original_numrecords=numrecords

/* look for multiple email addresses in each record and if found create a new record for each
   additional address that is found */

do i=1 to numrecords
   firstfield=0
   countemail=0

   do j=10 to 25       /* check all phone and custom fields */
       if j==15 then j=21
          fieldtype=j-5
          if ((j<15) & (pda.fieldtype.i==4) & CheckEmail(pda.j.i)) then call LookforAddresses()
          else
             do
                validemail=CheckEmail(pda.j.i)
                if (validemail==1) then call LookforAddresses()
             end
   end

end


say numrecords-original_numrecords 'new records created due to multiple email addresses found in data file'
say numrecords 'entries will be created in Yam address book'

/* sort the array by lastname */

say 'Sorting array...'
check = QuickSortStem(1,numrecords,LASTNAME)

say 'quicksort complete'

/* index array by category */

say 'Indexing array...'
do j=0 to catcount
    catnumrecords.j=0
end

do i=1 to numrecords
    do j=0 to catcount
      if pda.CATEGORY.i == j then
        do
          catnumrecords.j=catnumrecords.j+1
          indexcounter=catnumrecords.j
          pdaindex.j.indexcounter=i
        end
    end
end

say 'indexing complete'

/* now write to Yam addressbook  */

open('yam',destfile,'Write')

outfile = writeln('yam','YAB4 - YAM Addressbook')

do j = 0 to catcount
    outfile = writeln('yam','@GROUP '||catlongname.j)
    outfile = writeln('yam', catlongname.j)

   do i = 1 to catnumrecords.j
    indexcounter=pdaindex.j.i

/*    outfile = writeln('yam','@USER '||pda.FIRSTNAME.indexcounter||' '||pda.LASTNAME.indexcounter) */
    outfile = writeln('yam','@USER '||pda.ALIAS.indexcounter)

    /* look for email address */
    do k=5 to 9
        if(pda.k.indexcounter==4) then
          do
            emailadd=k
            leave
          end
    end
    eplus5 = emailadd + 5
    outfile = writeln('yam',pda.eplus5.indexcounter)
    outfile = writeln('yam',strip(pda.FIRSTNAME.indexcounter||' '||pda.LASTNAME.indexcounter,'L'))
    outfile = writeln('yam','')

    /* telephone numbers */

    telstr = ''
    do k=10 to 14
        if(pda.k.indexcounter~=='') then
           do
              kminus5 = k - 5
/*              if(kminus5~==emailadd) then  */
              if(pda.kminus5.indexcounter~==4) then
                 do
                     id=pda.kminus5.indexcounter
                     telstr = telstr||' '||pda.k.indexcounter||phonetype.id
                 end
           end
    end
    outfile = writeln('yam',strip(telstr,'L'))
    outfile = writeln('yam',space(translate(translate(pda.ADDRESSTEXT.indexcounter,' ',CR),' ',LF),1))
    outfile = writeln('yam',pda.CITY.indexcounter||' '||pda.ZIP.indexcounter)
    outfile = writeln('yam',pda.STATE.indexcounter)
    outfile = writeln('yam','')
    outfile = writeln('yam','')
    outfile = writeln('yam','')
    outfile = writeln('yam','')
    outfile = writeln('yam','')
    outfile = writeln('yam','@ENDUSER')

   end
   outfile = writeln('yam','@ENDGROUP')
end

close('yam')

exit

/* function to obtain string from CString data type */

GetCString:

    strlen = readch('adat',1)

    if (strlen=='FF'x) then
        do
            strlenTWO = c2d(reverse(readch('adat',2)))
            cstring = readch('adat',strlenTWO)
        end
    else
        do
            cstring = readch('adat',c2d(strlen))
        end

    return cstring

/* quicksort function */

QuickSortStem: procedure expose pda.

    arg nleft, nright, key
    i = nleft
    j = nright
    comp = (nleft + nright)%2

    x = pda.key.comp

    do while i<=j
       do while ((upper(pda.key.i)<upper(x)) & (i<nright))
          i=i+1
          end
       do while ((upper(pda.key.j)>upper(x)) & (j>nleft))
          j=j-1
          end

       if (i<=j) then
          do
            do k=1 to 26
              temp = pda.k.i
              pda.k.i = pda.k.j
              pda.k.j = temp
              end
            i=i+1
            j=j-1
          end
    end

    if (nleft<j) then check = QuickSortStem(nleft,j,key)
    
    if (i<nright) then check = QuickSortStem(i,nright,key)

  return '1'

/* this function checks a string to determine if it is a valid email address */

CheckEmail:

arg teststring
parse var teststring a '@' b '.' c
if ((a~='') & (b~='') & (c~='')) then return TRUE
else return FALSE

/* this function looks for additional email addresses */

LookforAddresses:

 if (firstfield==0) then
     do
        firstfield=j
        countemail=countemail+1
     end
 else
     do
        numrecords=numrecords+1
        countemail=countemail+1
        do k=1 to 26
          pda.k.numrecords = pda.k.i
        end
        pda.firstfield.numrecords=pda.j.i
/*        pda.LASTNAME.numrecords=pda.LASTNAME.numrecords||' ('||countemail||')' */
        pda.ALIAS.numrecords=pda.ALIAS.numrecords||' ('||countemail||')'

     end





