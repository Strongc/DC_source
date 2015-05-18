 /****************************************************************************/
 /*                                                                          */
 /*                                                                          */
 /*                                 GRUNDFOS                                 */
 /*                           DK-8850 BJERRINGBRO                            */
 /*                                 DENMARK                                  */
 /*               --------------------------------------------               */
 /*                Project: DC                                               */
 /*               --------------------------------------------               */
 /*                                                                          */
 /*               (C) Copyright Grundfos                                     */
 /*               All rights reserved                                        */
 /*               --------------------------------------------               */
 /*                                                                          */
 /*               As this is the  property of  GRUNDFOS  it                  */
 /*               must not be passed on to any person not aut-               */
 /*               horized  by GRUNDFOS or be  copied or other-               */
 /*               wise  utilized by anybody without GRUNDFOS'                */
 /*               expressed written permission.                              */
 /****************************************************************************/
 /* CLASS NAME       : Colours                                               */
 /*                                                                          */
 /* FILE NAME        : Colours.h                                             */
 /*                                                                          */
 /* CREATED DATE     : 2012-03-19                                            */
 /*                                                                          */
 /* SHORT FILE DESCRIPTION :                                                 */
 /****************************************************************************/
 /*****************************************************************************
    Protect against multiple inclusion through the use of guards:
  ****************************************************************************/
 #ifndef Colours_h
 #define Colours_h

 /*****************************************************************************
   SYSTEM INCLUDES
  *****************************************************************************/

 /*****************************************************************************
   PROJECT INCLUDES
  *****************************************************************************/

 /*****************************************************************************
   LOCAL INCLUDES
  ****************************************************************************/

 /*****************************************************************************
   DEFINES
  *****************************************************************************/

// Grundfos colours defined by Discovery & Design 3786
// Notice! EmWin uses colours as BGR (not RGB)
#define GF_DARKGRAY           0x3E3F41 //RGB 065.063.062
#define GF_LIGHTGRAY          0x595C5D //RGB 093.092.089
#define GF_GRAY               0x878787 //RGB 135.135.135
#define GF_GRAY_SHADOW        0x868686 //RGB 134.134.134
#define GF_TEXT_GRAY          0xB2B2B2 //RGB 178.178.178
#define GF_BLUE               0xD58F00 //RGB 000.143.213
#define GF_BLUE_HIGHLIGHT     0xFFC460 //RGB 096.196.255
#define GF_LIGHTBLUE          0xCC8800 //RGB 000.136.204

#define GF_BLUE100            0x854A06 // blue 100%
#define GF_BLUE065            0xB0895D // blue 65%
#define GF_BLUE040            0xCEB79B // blue 40%
#define GF_RED100             0x250B9F // red 100%
#define GF_RED065             0x7160C1 // red 65%
#define GF_RED040             0xA89DD9 // red 40%
#define GF_GREEN100           0x446E4E // green 100%
#define GF_GREEN065           0x85A18C // green 65%
#define GF_GREEN040           0xB4C5B8 // green 40%
#define GF_YELLOW100          0x1B85E0 // yellow 100%
#define GF_YELLOW065          0x6BB0EB // yellow 65%
#define GF_YELLOW040          0xA4CEF3 // yellow 40%

#ifdef TFT_16_BIT_LCD
//#define USE_TFT_COLOURS
#endif

#ifdef USE_STN_COLOURS
#undef USE_TFT_COLOURS
#endif


#ifdef USE_TFT_COLOURS

// Use any colour for TFT
  #define GUI_COLOUR_TEXT_DEFAULT_FOREGROUND     GF_TEXT_GRAY
  #define GUI_COLOUR_TEXT_INACTIVE_FOREGROUND    GF_GRAY
  #define GUI_COLOUR_TEXT_HEADLINE_FOREGROUND    GUI_WHITE
  #define GUI_COLOUR_TEXT_EDITMODE_FOREGROUND    GUI_GRAY
  #define GUI_COLOUR_DEFAULT_FOREGROUND          GUI_WHITE
  #define GUI_COLOUR_DEFAULT_BACKGROUND          GUI_BLACK
  #define GUI_COLOUR_FRAME_DEFAULT               GF_GRAY

  #define GUI_COLOUR_SELECTION_FRAME             GUI_WHITE
  #define GUI_COLOUR_SELECTION_FOREGROUND        GUI_WHITE
  #define GUI_COLOUR_SELECTION_BACKGROUND        GUI_COLOUR_DEFAULT_BACKGROUND

  #define GUI_COLOUR_UPPER_STATUSLINE_FOREGROUND GUI_WHITE
  #define GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND GUI_BLACK
  #define GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND GF_GRAY
  #define GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND GUI_BLACK
  
  #define GUI_COLOUR_INFOLINE_FOREGROUND         GUI_COLOUR_TEXT_HEADLINE_FOREGROUND
  #define GUI_COLOUR_INFOLINE_BACKGROUND         GUI_COLOUR_DEFAULT_BACKGROUND

  #define GUI_COLOUR_BAR_DEFAULT                 GF_BLUE_HIGHLIGHT

  #define GUI_COLOUR_SLIDER_INACTIVE             GUI_WHITE
  #define GUI_COLOUR_SLIDER_ACTIVE               GUI_WHITE

  #define GUI_COLOUR_TAB_FOREGROUND              GF_GRAY
  #define GUI_COLOUR_TAB_BACKGROUND              GF_DARKGRAY
  #define GUI_COLOUR_TAB_SELECTED_FOREGROUND     GUI_COLOUR_UPPER_STATUSLINE_FOREGROUND
  #define GUI_COLOUR_TAB_SELECTED_BACKGROUND     GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND

  #define GUI_COLOUR_GRAPH_DEFAULT               GF_BLUE100

  #define GUI_COLOUR_GRAPH_1_FORE                GUI_COLOUR_TEXT_HEADLINE_FOREGROUND
  #define GUI_COLOUR_GRAPH_1_BACK                GF_BLUE100
  #define GUI_COLOUR_GRAPH_2_FORE                GUI_COLOUR_TEXT_HEADLINE_FOREGROUND
  #define GUI_COLOUR_GRAPH_2_BACK                GF_GREEN100
  #define GUI_COLOUR_GRAPH_3_FORE                GUI_COLOUR_TEXT_HEADLINE_FOREGROUND
  #define GUI_COLOUR_GRAPH_3_BACK                GF_YELLOW100

#else

// Use grayscale colours for STN
  #define GUI_COLOUR_TEXT_DEFAULT_FOREGROUND     GUI_BLACK
  #define GUI_COLOUR_TEXT_INACTIVE_FOREGROUND    GF_GRAY
  #define GUI_COLOUR_TEXT_HEADLINE_FOREGROUND    GUI_BLACK
  #define GUI_COLOUR_TEXT_EDITMODE_FOREGROUND    GUI_GRAY
  #define GUI_COLOUR_DEFAULT_FOREGROUND          GUI_BLACK
  #define GUI_COLOUR_DEFAULT_BACKGROUND          GUI_WHITE 
  #define GUI_COLOUR_FRAME_DEFAULT               GUI_BLACK

  #define GUI_COLOUR_SELECTION_FRAME             GUI_BLACK
  #define GUI_COLOUR_SELECTION_FOREGROUND        GUI_BLACK
  #define GUI_COLOUR_SELECTION_BACKGROUND        GUI_WHITE

  #define GUI_COLOUR_UPPER_STATUSLINE_FOREGROUND GUI_WHITE
  #define GUI_COLOUR_UPPER_STATUSLINE_BACKGROUND GUI_BLACK
  #define GUI_COLOUR_LOWER_STATUSLINE_FOREGROUND GUI_WHITE
  #define GUI_COLOUR_LOWER_STATUSLINE_BACKGROUND GUI_BLACK

  #define GUI_COLOUR_INFOLINE_FOREGROUND         GUI_WHITE
  #define GUI_COLOUR_INFOLINE_BACKGROUND         GUI_DARKGRAY

  #define GUI_COLOUR_BAR_DEFAULT                 GUI_GRAY

  #define GUI_COLOUR_SLIDER_INACTIVE             GUI_DARKGRAY
  #define GUI_COLOUR_SLIDER_ACTIVE               GUI_DARKGRAY

  #define GUI_COLOUR_TAB_FOREGROUND              GUI_WHITE
  #define GUI_COLOUR_TAB_BACKGROUND              GUI_DARKGRAY
  #define GUI_COLOUR_TAB_SELECTED_FOREGROUND     GUI_BLACK
  #define GUI_COLOUR_TAB_SELECTED_BACKGROUND     GUI_WHITE

  #define GUI_COLOUR_GRAPH_DEFAULT               GUI_BLACK

  #define GUI_COLOUR_GRAPH_1_FORE                GUI_BLACK
  #define GUI_COLOUR_GRAPH_1_BACK                GUI_GRAY
  #define GUI_COLOUR_GRAPH_2_FORE                GUI_BLACK
  #define GUI_COLOUR_GRAPH_2_BACK                GUI_LIGHTGRAY
  #define GUI_COLOUR_GRAPH_3_FORE                GUI_WHITE
  #define GUI_COLOUR_GRAPH_3_BACK                GUI_DARKGRAY
#endif
    
  
 /*****************************************************************************
   TYPE DEFINES
  *****************************************************************************/
 #endif
