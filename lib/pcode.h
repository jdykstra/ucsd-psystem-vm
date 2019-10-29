/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2010 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef LIB_PCODE_H
#define LIB_PCODE_H

#define SLDC_0 0                        /* Short LoaD Constant 0 */
#define SLDC_1 1                        /* Short LoaD Constant 1 */
#define SLDC_2 2                        /* Short LoaD Constant 2 */
#define SLDC_3 3                        /* Short LoaD Constant 3 */
#define SLDC_4 4                        /* Short LoaD Constant 4 */
#define SLDC_5 5                        /* Short LoaD Constant 5 */
#define SLDC_6 6                        /* Short LoaD Constant 6 */
#define SLDC_7 7                        /* Short LoaD Constant 7 */
#define SLDC_8 8                        /* Short LoaD Constant 8 */
#define SLDC_9 9                        /* Short LoaD Constant 9 */
#define SLDC_10 10                      /* Short LoaD Constant 10 */
#define SLDC_11 11                      /* Short LoaD Constant 11 */
#define SLDC_12 12                      /* Short LoaD Constant 12 */
#define SLDC_13 13                      /* Short LoaD Constant 13 */
#define SLDC_14 14                      /* Short LoaD Constant 14 */
#define SLDC_15 15                      /* Short LoaD Constant 15 */
#define SLDC_16 16                      /* Short LoaD Constant 16 */
#define SLDC_17 17                      /* Short LoaD Constant 17 */
#define SLDC_18 18                      /* Short LoaD Constant 18 */
#define SLDC_19 19                      /* Short LoaD Constant 19 */
#define SLDC_20 20                      /* Short LoaD Constant 20 */
#define SLDC_21 21                      /* Short LoaD Constant 21 */
#define SLDC_22 22                      /* Short LoaD Constant 22 */
#define SLDC_23 23                      /* Short LoaD Constant 23 */
#define SLDC_24 24                      /* Short LoaD Constant 24 */
#define SLDC_25 25                      /* Short LoaD Constant 25 */
#define SLDC_26 26                      /* Short LoaD Constant 26 */
#define SLDC_27 27                      /* Short LoaD Constant 27 */
#define SLDC_28 28                      /* Short LoaD Constant 28 */
#define SLDC_29 29                      /* Short LoaD Constant 29 */
#define SLDC_30 30                      /* Short LoaD Constant 30 */
#define SLDC_31 31                      /* Short LoaD Constant 31 */
#define SLDC_32 32                      /* Short LoaD Constant 32 */
#define SLDC_33 33                      /* Short LoaD Constant 33 */
#define SLDC_34 34                      /* Short LoaD Constant 34 */
#define SLDC_35 35                      /* Short LoaD Constant 35 */
#define SLDC_36 36                      /* Short LoaD Constant 36 */
#define SLDC_37 37                      /* Short LoaD Constant 37 */
#define SLDC_38 38                      /* Short LoaD Constant 38 */
#define SLDC_39 39                      /* Short LoaD Constant 39 */
#define SLDC_40 40                      /* Short LoaD Constant 40 */
#define SLDC_41 41                      /* Short LoaD Constant 41 */
#define SLDC_42 42                      /* Short LoaD Constant 42 */
#define SLDC_43 43                      /* Short LoaD Constant 43 */
#define SLDC_44 44                      /* Short LoaD Constant 44 */
#define SLDC_45 45                      /* Short LoaD Constant 45 */
#define SLDC_46 46                      /* Short LoaD Constant 46 */
#define SLDC_47 47                      /* Short LoaD Constant 47 */
#define SLDC_48 48                      /* Short LoaD Constant 48 */
#define SLDC_49 49                      /* Short LoaD Constant 49 */
#define SLDC_50 50                      /* Short LoaD Constant 50 */
#define SLDC_51 51                      /* Short LoaD Constant 51 */
#define SLDC_52 52                      /* Short LoaD Constant 52 */
#define SLDC_53 53                      /* Short LoaD Constant 53 */
#define SLDC_54 54                      /* Short LoaD Constant 54 */
#define SLDC_55 55                      /* Short LoaD Constant 55 */
#define SLDC_56 56                      /* Short LoaD Constant 56 */
#define SLDC_57 57                      /* Short LoaD Constant 57 */
#define SLDC_58 58                      /* Short LoaD Constant 58 */
#define SLDC_59 59                      /* Short LoaD Constant 59 */
#define SLDC_60 60                      /* Short LoaD Constant 60 */
#define SLDC_61 61                      /* Short LoaD Constant 61 */
#define SLDC_62 62                      /* Short LoaD Constant 62 */
#define SLDC_63 63                      /* Short LoaD Constant 63 */
#define SLDC_64 64                      /* Short LoaD Constant 64 */
#define SLDC_65 65                      /* Short LoaD Constant 65 */
#define SLDC_66 66                      /* Short LoaD Constant 66 */
#define SLDC_67 67                      /* Short LoaD Constant 67 */
#define SLDC_68 68                      /* Short LoaD Constant 68 */
#define SLDC_69 69                      /* Short LoaD Constant 69 */
#define SLDC_70 70                      /* Short LoaD Constant 70 */
#define SLDC_71 71                      /* Short LoaD Constant 71 */
#define SLDC_72 72                      /* Short LoaD Constant 72 */
#define SLDC_73 73                      /* Short LoaD Constant 73 */
#define SLDC_74 74                      /* Short LoaD Constant 74 */
#define SLDC_75 75                      /* Short LoaD Constant 75 */
#define SLDC_76 76                      /* Short LoaD Constant 76 */
#define SLDC_77 77                      /* Short LoaD Constant 77 */
#define SLDC_78 78                      /* Short LoaD Constant 78 */
#define SLDC_79 79                      /* Short LoaD Constant 79 */
#define SLDC_80 80                      /* Short LoaD Constant 80 */
#define SLDC_81 81                      /* Short LoaD Constant 81 */
#define SLDC_82 82                      /* Short LoaD Constant 82 */
#define SLDC_83 83                      /* Short LoaD Constant 83 */
#define SLDC_84 84                      /* Short LoaD Constant 84 */
#define SLDC_85 85                      /* Short LoaD Constant 85 */
#define SLDC_86 86                      /* Short LoaD Constant 86 */
#define SLDC_87 87                      /* Short LoaD Constant 87 */
#define SLDC_88 88                      /* Short LoaD Constant 88 */
#define SLDC_89 89                      /* Short LoaD Constant 89 */
#define SLDC_90 90                      /* Short LoaD Constant 90 */
#define SLDC_91 91                      /* Short LoaD Constant 91 */
#define SLDC_92 92                      /* Short LoaD Constant 92 */
#define SLDC_93 93                      /* Short LoaD Constant 93 */
#define SLDC_94 94                      /* Short LoaD Constant 94 */
#define SLDC_95 95                      /* Short LoaD Constant 95 */
#define SLDC_96 96                      /* Short LoaD Constant 96 */
#define SLDC_97 97                      /* Short LoaD Constant 97 */
#define SLDC_98 98                      /* Short LoaD Constant 98 */
#define SLDC_99 99                      /* Short LoaD Constant 99 */
#define SLDC_100 100                    /* Short LoaD Constant 100 */
#define SLDC_101 101                    /* Short LoaD Constant 101 */
#define SLDC_102 102                    /* Short LoaD Constant 102 */
#define SLDC_103 103                    /* Short LoaD Constant 103 */
#define SLDC_104 104                    /* Short LoaD Constant 104 */
#define SLDC_105 105                    /* Short LoaD Constant 105 */
#define SLDC_106 106                    /* Short LoaD Constant 106 */
#define SLDC_107 107                    /* Short LoaD Constant 107 */
#define SLDC_108 108                    /* Short LoaD Constant 108 */
#define SLDC_109 109                    /* Short LoaD Constant 109 */
#define SLDC_110 110                    /* Short LoaD Constant 110 */
#define SLDC_111 111                    /* Short LoaD Constant 111 */
#define SLDC_112 112                    /* Short LoaD Constant 112 */
#define SLDC_113 113                    /* Short LoaD Constant 113 */
#define SLDC_114 114                    /* Short LoaD Constant 114 */
#define SLDC_115 115                    /* Short LoaD Constant 115 */
#define SLDC_116 116                    /* Short LoaD Constant 116 */
#define SLDC_117 117                    /* Short LoaD Constant 117 */
#define SLDC_118 118                    /* Short LoaD Constant 118 */
#define SLDC_119 119                    /* Short LoaD Constant 119 */
#define SLDC_120 120                    /* Short LoaD Constant 120 */
#define SLDC_121 121                    /* Short LoaD Constant 121 */
#define SLDC_122 122                    /* Short LoaD Constant 122 */
#define SLDC_123 123                    /* Short LoaD Constant 123 */
#define SLDC_124 124                    /* Short LoaD Constant 124 */
#define SLDC_125 125                    /* Short LoaD Constant 125 */
#define SLDC_126 126                    /* Short LoaD Constant 126 */
#define SLDC_127 127                    /* Short LoaD Constant 127 */
#define ABI     128                     /* ABsolute Integer */
#define ABR     129                     /* ABsolure Real */
#define ADI     130                     /* ADd Integer */
#define ADR     131                     /* ADd Real */
#define LAND    132                     /* Logical AND */
#define DIF     133                     /* set DIFference */
#define DVI     134                     /* DiVide Integer */
#define DVR     135                     /* DiVide Real */
#define CHK     136                     /* CHecK */
#define FLO     137                     /* FLoat next to tOs */
#define FLT     138                     /* FLoat Tos */
#define INN     139                     /* set IN operation */
#define INT     140                     /* set INTersection */
#define LOR     141                     /* Logical OR */
#define MODI    142                     /* MODulo Integer */
#define MPI     143                     /* MultiPly Integer */
#define MPR     144                     /* MultiPly Real */
#define NGI     145                     /* NeGate Integer */
#define NGR     146                     /* NeGate Real */
#define LNOT    147                     /* Logical NOT */
#define SRS     148                     /* SubRange Set */
#define SBI     149                     /* SuBstract Integer */
#define SBR     150                     /* SiBstract Real */
#define SGS     151                     /* SinGleton Set */
#define SQI     152                     /* SQuare Integer */
#define SQR     153                     /* SQare Real */
#define STO     154                     /* STOre indirect */
#define IXS     155                     /* IndeX String array */
#define UNI     156                     /* set UNIon */
#define LDE     157                     /* LoaD Extended */
#define CSP     158                     /* Call Standard Procedure */
#define LDCN    159                     /* LoaD Constant Nil */
#define ADJ     160                     /* ADJust set */
#define FJP     161                     /* False JumP */
#define INC     162                     /* INCrement field pointer */
#define IND     163                     /* INdex and loaD */
#define IXA     164                     /* IndeX Array */
#define LAO     165                     /* Load glObal Address */
#define LSA     166                     /* Load String Address */
#define LAE     167                     /* Load Address Extended */
#define MOV     168                     /* MOVe words */
#define LDO     169                     /* LoaD glObal */
#define SAS     170                     /* String ASsign */
#define SRO     171                     /* StoRe glObal */
#define XJP     172                     /* case JumP */
#define RNP     173                     /* Return from Non-base Procedure */
#define CIP     174                     /* Call Intermeduiate Procedure */
#define EQU     175                     /* EQUal */
#define GEQ     176                     /* Greater or EQual */
#define GRT     177                     /* GReaTer */
#define LDA     178                     /* LoaD intermediate Address */
#define LDC     179                     /* LoaD multiple word Constant */
#define LEQ     180                     /* Less or EQual */
#define LES     181                     /* LESs */
#define LOD     182                     /* LOaD intermediate */
#define NEQ     183                     /* Not EQual */
#define STR     184                     /* SToRe intermediate */
#define UJP     185                     /* Unconditional JumP */
#define LDP     186                     /* LoaD Packed field */
#define STP     187                     /* STore Packed field */
#define LDM     188                     /* LoaD Multiple words */
#define STM     189                     /* STore Multiple words */
#define LDB     190                     /* LoaD Byte */
#define STB     191                     /* STore Byte */
#define IXP     192                     /* IndeX Packed array */
#define RBP     193                     /* Return from Base Procedure */
#define CBP     194                     /* Call Base Procedure */
#define EQUI    195                     /* EQUal Integer */
#define GEQI    196                     /* Greater or EQual Integer */
#define GRTI    197                     /* GReaTer Integer */
#define LLA     198                     /* Load Local Address */
#define LDCI    199                     /* LoaD Constant Integer */
#define LEQI    200                     /* Less or EQual Integer */
#define LESI    201                     /* LESs Integer */
#define LDL     202                     /* LoaD Local */
#define NEQI    203                     /* Not EQual Integer */
#define STL     204                     /* STore Local */
#define CXP     205                     /* Call eXternal Procedure */
#define CLP     206                     /* Call Local Procedure */
#define CGP     207                     /* Call Global Procedure */
#define LPA     208                     /* Load Packed Array */
#define STE     209                     /* STore Extended */

#define EFJ     211                     /* Equal False Jump */
#define NFJ     212                     /* Not equal False Jump */
#define BPT     213                     /* BreakPoinT */
#define XIT     214                     /* eXIT */
#define NOP     215                     /* No OPeration */
#define SLDL_1  216                     /* Short LoaD Local 1 */
#define SLDL_2  217                     /* Short LoaD Local 2 */
#define SLDL_3  218                     /* Short LoaD Local 3 */
#define SLDL_4  219                     /* Short LoaD Local 4 */
#define SLDL_5  220                     /* Short LoaD Local 5 */
#define SLDL_6  221                     /* Short LoaD Local 6 */
#define SLDL_7  222                     /* Short LoaD Local 7 */
#define SLDL_8  223                     /* Short LoaD Local 8 */
#define SLDL_9  224                     /* Short LoaD Local 9 */
#define SLDL_10 225                     /* Short LoaD Local 10 */
#define SLDL_11 226                     /* Short LoaD Local 11 */
#define SLDL_12 227                     /* Short LoaD Local 12 */
#define SLDL_13 228                     /* Short LoaD Local 13 */
#define SLDL_14 229                     /* Short LoaD Local 14 */
#define SLDL_15 230                     /* Short LoaD Local 15 */
#define SLDL_16 231                     /* Short LoaD Local 16 */
#define SLDO_1  232                     /* Short LoaD glObal 1 */
#define SLDO_2  233                     /* Short LoaD glObal 2 */
#define SLDO_3  234                     /* Short LoaD glObal 3 */
#define SLDO_4  235                     /* Short LoaD glObal 4 */
#define SLDO_5  236                     /* Short LoaD glObal 5 */
#define SLDO_6  237                     /* Short LoaD glObal 6 */
#define SLDO_7  238                     /* Short LoaD glObal 7 */
#define SLDO_8  239                     /* Short LoaD glObal 8 */
#define SLDO_9  240                     /* Short LoaD glObal 9 */
#define SLDO_10 241                     /* Short LoaD glObal 10 */
#define SLDO_11 242                     /* Short LoaD glObal 11 */
#define SLDO_12 243                     /* Short LoaD glObal 12 */
#define SLDO_13 244                     /* Short LoaD glObal 13 */
#define SLDO_14 245                     /* Short LoaD glObal 14 */
#define SLDO_15 246                     /* Short LoaD glObal 15 */
#define SLDO_16 247                     /* Short LoaD glObal 16 */
#define SIND_0  248                     /* Short load INDirect */
#define SIND_1  249                     /* Short INdex 1 and loaD  */
#define SIND_2  250                     /* Short INdex 2 and loaD  */
#define SIND_3  251                     /* Short INdex 3 and loaD  */
#define SIND_4  252                     /* Short INdex 4 and loaD  */
#define SIND_5  253                     /* Short INdex 5 and loaD  */
#define SIND_6  254                     /* Short INdex 6 and loaD  */
#define SIND_7  255                     /* Short INdex 7 and loaD  */

#define CSP_IOC         0               /* IoCheck() */
#define CSP_NEW         1               /* new() */
#define CSP_MVL         2               /* moveleft() */
#define CSP_MVR         3               /* moveright() */
#define CSP_XIT         4               /* exit() */
#define CSP_UREAD       5               /* unitread() */
#define CSP_UWRITE      6               /* unitwrite() */
#ifndef APPLE_1_3
#define CSP_IDS         7               /* idsearch() */
#define CSP_TRS         8               /* treesearch() */
#endif
#define CSP_TIM         9               /* time() */
#define CSP_FLC         10              /* fillchar() */
#define CSP_SCN         11              /* scan() */
#define CSP_USTAT       12              /* unitstat() */

#define CSP_LDSEG       21              /* LoadSegment() */
#define CSP_ULDSEG      22              /* UnloadSegment() */
#define CSP_TRC         23              /* trunc() */
#define CSP_RND         24              /* round() */
#define CSP_SIN         25              /* sin() */
#define CSP_COS         26              /* cos() */
#define CSP_TAN         27              /* tan() */
#define CSP_ATAN        28              /* atan() */
#define CSP_LN          29              /* ln() */
#define CSP_EXP         30              /* exp() */
#define CSP_SQRT        31              /* sqrt() */
#define CSP_MRK         32              /* mark() */
#define CSP_RLS         33              /* release() */
#define CSP_IOR         34              /* ioresult() */
#define CSP_UBUSY       35              /* unitbusy() */
#define CSP_POT         36              /* PwrOfTen() */
#define CSP_UWAIT       37              /* unitwait() */
#define CSP_UCLEAR      38              /* unitclear() */
#define CSP_HLT         39              /* halt() */
#define CSP_MAV         40              /* memavail() */

#define XSYSERR         0
#define XINVNDX         1               /* invalid index */
#define XNOPROC         2               /* no segment */
#define XNOEXIT         3               /* exit from uncalled procedure */
#define XSTKOVR         4               /* stack overflow */
#define XINTOVR         5               /* integer overflow */
#define XDIVZER         6               /* divide by zero */
#define XBADMEM         7               /* invalid memory reference */
#define XUBREAK         8               /* user break */
#define XSYIOER         9               /* system IO error */
#define XUIOERR         10              /* user IO error */
#define XNOTIMP         11              /* unimplemented instruction */
#define XFPIERR         12              /* floating point math error */
#define XS2LONG         13              /* string to long */
#define XHLTBPT         14              /* Halt, Breakpoing */
#define XBRKPNT         15              /* Breakpoint */

#endif /* LIB_PCODE_H */
