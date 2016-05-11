#/bin/sh
#
# A simple shell script to generate the appropriate zoneinfo files we require
# for YAM.
#

ZIC=zic
CP=cp
RM=rm
TZLIB="src/tools/tz"
TZDIR="../../../dist/common/YAM/Resources/Zoneinfo/"

if [ ! -e "${TZLIB}" ]; then
  echo "ERROR: You have to call this script from the top-level directory."
  exit 2
fi

# change in TZLIB
cd ${TZLIB}

# before starting we clear TZDIR
${RM} -rf ${TZDIR}/*

# use 'zic' to generate the zoneinfo files we require for yam
TIMEZONES="africa antarctica asia australasia europe northamerica southamerica"
for tz in ${TIMEZONES}; do
  ${ZIC} -y yearistype.sh -d ${TZDIR} -v ${tz}
done

# copy the .tab files because YAM will use them
${CP} zone1970.tab ${TZDIR}/zone.tab

# remove some backward compatibility files the zic compiler created
# but yam will not use
RMFILES="CET CST6CDT EET EST EST5EDT HST MET MST MST7MDT PST8PDT WET"
for file in ${RMFILES}; do
  ${RM} ${TZDIR}/${file}
done
