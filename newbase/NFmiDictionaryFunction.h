
/*!  NFmiDictionaryFunction.h
 * Tämä on Editorin käyttämä sana kirja funktio.
 * Kieli versiot stringeihin tulevat täältä.
 */

#pragma once

#include <string>

// HUOM! Tämä on kopio NFmiEditMapGeneralDataDoc-luokan metodista, kun en voinut antaa tänne
// dokumenttia
std::string GetDictionaryString(const char *theMagicWord);
