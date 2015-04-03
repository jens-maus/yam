#/bin/sh
#
# A simple shell script to generate the appropriate zoneinfo files we require
# for YAM.
#

ZIC=zic
CP=cp
RM=rm
TZDIR="../../resources/zoneinfo/"

# before starting we clear TZDIR
${RM} -rf ${TZDIR}/*

# use 'zic' to generate the zoneinfo files we require for yam
TIMEZONES="africa antarctica asia australasia europe northamerica southamerica"
for tz in ${TIMEZONES}; do
  ${ZIC} -y tzdata/yearistype.sh -d ${TZDIR} -v tzdata/${tz}
done

# copy the .tab files because YAM will use them
${CP} tzdata/zone1970.tab ${TZDIR}/zone.tab

# remove some backward compatibility files the zic compiler created
# but yam will not use
RMFILES="CET CST6CDT EET EST EST5EDT HST MET MST MST7MDT PST8PDT WET"
for file in ${RMFILES}; do
  ${RM} ${TZDIR}/${file}
done
