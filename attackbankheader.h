#pragma once

enum XACT_WAVEBANK_ATTACKBANK : unsigned int
{
    XACT_WAVEBANK_ATTACKBANK_GRAB = 0,
    XACT_WAVEBANK_ATTACKBANK_FTHROW = 1,
    XACT_WAVEBANK_ATTACKBANK_UPTHROW = 2,
    XACT_WAVEBANK_ATTACKBANK_BACKTHROW = 3,
    XACT_WAVEBANK_ATTACKBANK_DOWNTHROW = 4,
    XACT_WAVEBANK_ATTACKBANK_GETUPATTACK = 5,
    XACT_WAVEBANK_ATTACKBANK_JAB = 6,
    XACT_WAVEBANK_ATTACKBANK_FTILT = 7,
    XACT_WAVEBANK_ATTACKBANK_UPTILT = 8,
    XACT_WAVEBANK_ATTACKBANK_DTILT = 9,
    XACT_WAVEBANK_ATTACKBANK_FSMASH = 10,
    XACT_WAVEBANK_ATTACKBANK_UPSMASH = 11,
    XACT_WAVEBANK_ATTACKBANK_DSMASH = 12,
    XACT_WAVEBANK_ATTACKBANK_NAIR = 13,
    XACT_WAVEBANK_ATTACKBANK_FAIR = 14,
    XACT_WAVEBANK_ATTACKBANK_UPAIR = 15,
    XACT_WAVEBANK_ATTACKBANK_DAIR = 16,
    XACT_WAVEBANK_ATTACKBANK_BAIR = 17,
    XACT_WAVEBANK_ATTACKBANK_NEUTRALB = 18,
    XACT_WAVEBANK_ATTACKBANK_SIDEB = 19,
    XACT_WAVEBANK_ATTACKBANK_UPB = 20,
    XACT_WAVEBANK_ATTACKBANK_DOWNB = 21,
    XACT_WAVEBANK_ATTACKBANK_WEAK = 22,
    XACT_WAVEBANK_ATTACKBANK_MEDIUM = 23,
    XACT_WAVEBANK_ATTACKBANK_HEAVY = 24,
};

#define XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT 25