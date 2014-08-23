//NOTE: longitude is positive for East and negative for West
//      Milan, Coordinates :  45.4667° N, 9.1833° E 
#define LATITUDE    45.4667
#define LONGITUDE 9.1833
#define TIMEZONE +1
#define DAY_NAME_LANGUAGE DAY_NAME_ENGLISH 				// Valid values: DAY_NAME_ENGLISH, DAY_NAME_GERMAN, DAY_NAME_FRENCH
#define MOONPHASE_NAME_LANGUAGE MOONPHASE_TEXT_ENGLISH 	// Valid values: MOONPHASE_TEXT_ENGLISH, MOONPHASE_TEXT_GERMAN, MOONPHASE_TEXT_FRENCH
#define day_month_x day_month_day_first 				// Valid values: day_month_month_first, day_month_day_first
#define TRANSLATION_CW "WK%V" 							// Translation for the calendar week (e.g. "CW%V")

// ----- Additional time zones to display on the top right 
//      -NOTE: if timezone is < -24 then that timezone won't be shown
#define AdditionalTimezone_1 +6 						// Timezone offest
#define AdditionalTimezone_2 -999 						// Timezone offest (NY = -6)
#define AdditionalTimezone_3 -999 						// Timezone offest (Syd = -8)
static char AdditionalTimezone_1_Description[] = "Sg"; // Timezone name to display
static char AdditionalTimezone_2_Description[] = "SY"; // Timezone name to display
static char AdditionalTimezone_3_Description[] = "Syd"; // Timezone name to display
// ----- Additional time zones to display on the top right


// ---- Constants for all available languages ----------------------------------------

const char *MOONPHASE_TEXT_GERMAN[] = {
	"NM",
	"NM+",
	"NM++",
	"VM-",
	"VM",
	"VM+",
	"VM++",
	"NM-"
};

const char *DAY_NAME_GERMAN[] = {
	"SON",
	"MON",
	"DIE",
	"MIT",
	"DON",
	"FRE",
	"SAM"
};

const char *DAY_NAME_ENGLISH[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

const char *MOONPHASE_TEXT_ENGLISH[] = {
	"",
	"+",
	"+",
	"+",
	"",
	"-",
	"-",
	"-"
};

const char *DAY_NAME_FRENCH[] = {
    "DIM",
    "LUN",
    "MAR",
    "MER",
    "JEU",
    "VEN",
    "SAM"
};
const char *MOONPHASE_TEXT_FRENCH[] = {
    "NL",
    "NL+",
    "NL++",
    "PL-",
    "PL",
    "PL+",
    "PL++",
    "NL-"
};

// the following are used in the define section (only one of them is used)
const int day_month_day_first[] = {
	75,
	108
};

const int day_month_month_first[] = {
	108,
	75
};
