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
#define AdditionalTimezone_1 +6 						// Timezone offest
#define AdditionalTimezone_2 +8 						// Timezone offest
static char AdditionalTimezone_1_Description[] = "Sg"; // Timezone name to display
static char AdditionalTimezone_2_Description[] = "Syd"; // Timezone name to display
// ----- Additional time zones to display on the top right


// ---- Constants for all available languages ----------------------------------------

const int day_month_day_first[] = {
	75,
	108
};

const int day_month_month_first[] = {
	108,
	75
};

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
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT"
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
