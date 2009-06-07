#ifndef PRAGMAS_AEXPAT_PRAGMAS_H
#define PRAGMAS_AEXPAT_PRAGMAS_H

#ifndef CLIB_AEXPAT_PROTOS_H
#include <clib/expat_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(ExpatBase,0x01e,XML_ParserCreate(a0))
#pragma amicall(ExpatBase,0x024,XML_ParserCreateNS(a0,d0))
#pragma amicall(ExpatBase,0x02a,XML_ParserCreate_MM(a0,a1,a2))
#pragma amicall(ExpatBase,0x030,XML_ExternalEntityParserCreate(a0,a1,a2))
#pragma amicall(ExpatBase,0x036,XML_ParserFree(a0))
#pragma amicall(ExpatBase,0x03c,XML_Parse(a0,a1,d0,d1))
#pragma amicall(ExpatBase,0x042,XML_ParseBuffer(a0,d0,d1))
#pragma amicall(ExpatBase,0x048,XML_GetBuffer(a0,d0))
#pragma amicall(ExpatBase,0x04e,XML_SetStartElementHandler(a0,a1))
#pragma amicall(ExpatBase,0x054,XML_SetEndElementHandler(a0,a1))
#pragma amicall(ExpatBase,0x05a,XML_SetElementHandler(a0,a1,a2))
#pragma amicall(ExpatBase,0x060,XML_SetCharacterDataHandler(a0,a1))
#pragma amicall(ExpatBase,0x066,XML_SetProcessingInstructionHandler(a0,a1))
#pragma amicall(ExpatBase,0x06c,XML_SetCommentHandler(a0,a1))
#pragma amicall(ExpatBase,0x072,XML_SetStartCdataSectionHandler(a0,a1))
#pragma amicall(ExpatBase,0x078,XML_SetEndCdataSectionHandler(a0,a1))
#pragma amicall(ExpatBase,0x07e,XML_SetCdataSectionHandler(a0,a1,a2))
#pragma amicall(ExpatBase,0x084,XML_SetDefaultHandler(a0,a1))
#pragma amicall(ExpatBase,0x08a,XML_SetDefaultHandlerExpand(a0,a1))
#pragma amicall(ExpatBase,0x090,XML_SetExternalEntityRefHandler(a0,a1))
#pragma amicall(ExpatBase,0x096,XML_SetExternalEntityRefHandlerArg(a0,a1))
#pragma amicall(ExpatBase,0x09c,XML_SetUnknownEncodingHandler(a0,a1,a2))
#pragma amicall(ExpatBase,0x0a2,XML_SetStartNamespaceDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0a8,XML_SetEndNamespaceDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0ae,XML_SetNamespaceDeclHandler(a0,a1,a2))
#pragma amicall(ExpatBase,0x0b4,XML_SetXmlDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0ba,XML_SetStartDoctypeDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0c0,XML_SetEndDoctypeDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0c6,XML_SetDoctypeDeclHandler(a0,a1,a2))
#pragma amicall(ExpatBase,0x0cc,XML_SetElementDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0d2,XML_SetAttlistDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0d8,XML_SetEntityDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0de,XML_SetUnparsedEntityDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0e4,XML_SetNotationDeclHandler(a0,a1))
#pragma amicall(ExpatBase,0x0ea,XML_SetNotStandaloneHandler(a0,a1))
#pragma amicall(ExpatBase,0x0f0,XML_GetErrorCode(a0))
#pragma amicall(ExpatBase,0x0f6,XML_ErrorString(d0))
#pragma amicall(ExpatBase,0x0fc,XML_GetCurrentByteIndex(a0))
#pragma amicall(ExpatBase,0x102,XML_GetCurrentLineNumber(a0))
#pragma amicall(ExpatBase,0x108,XML_GetCurrentColumnNumber(a0))
#pragma amicall(ExpatBase,0x10e,XML_GetCurrentByteCount(a0))
#pragma amicall(ExpatBase,0x114,XML_GetInputContext(a0,a1,a2))
#pragma amicall(ExpatBase,0x11a,XML_SetUserData(a0,a1))
#pragma amicall(ExpatBase,0x120,XML_DefaultCurrent(a0))
#pragma amicall(ExpatBase,0x126,XML_UseParserAsHandlerArg(a0))
#pragma amicall(ExpatBase,0x12c,XML_SetBase(a0,a1))
#pragma amicall(ExpatBase,0x132,XML_GetBase(a0))
#pragma amicall(ExpatBase,0x138,XML_GetSpecifiedAttributeCount(a0))
#pragma amicall(ExpatBase,0x13e,XML_GetIdAttributeIndex(a0))
#pragma amicall(ExpatBase,0x144,XML_SetEncoding(a0,a1))
#pragma amicall(ExpatBase,0x14a,XML_SetParamEntityParsing(a0,d0))
#pragma amicall(ExpatBase,0x150,XML_SetReturnNSTriplet(a0,d0))
#pragma amicall(ExpatBase,0x156,XML_ExpatVersion())
#pragma amicall(ExpatBase,0x15c,XML_ExpatVersionInfo())
#pragma amicall(ExpatBase,0x162,XML_ParserReset(a0,a1))
#pragma amicall(ExpatBase,0x168,XML_SetSkippedEntityHandler(a0,a1))
#pragma amicall(ExpatBase,0x16e,XML_UseForeignDTD(a0,d0))
#pragma amicall(ExpatBase,0x174,XML_GetFeatureList())
#pragma amicall(ExpatBase,0x17a,XML_StopParser(a0,d0))
#pragma amicall(ExpatBase,0x180,XML_ResumeParser(a0))
#pragma amicall(ExpatBase,0x186,XML_GetParsingStatus(a0,a1))
#pragma amicall(ExpatBase,0x18c,XML_FreeContentModel(a0,a1))
#pragma amicall(ExpatBase,0x192,XML_MemMalloc(a0,d0))
#pragma amicall(ExpatBase,0x198,XML_MemRealloc(a0,a1,d0))
#pragma amicall(ExpatBase,0x19e,XML_MemFree(a0,a1))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall ExpatBase XML_ParserCreate       01e 801
#pragma  libcall ExpatBase XML_ParserCreateNS     024 0802
#pragma  libcall ExpatBase XML_ParserCreate_MM    02a a9803
#pragma  libcall ExpatBase XML_ExternalEntityParserCreate 030 a9803
#pragma  libcall ExpatBase XML_ParserFree         036 801
#pragma  libcall ExpatBase XML_Parse              03c 109804
#pragma  libcall ExpatBase XML_ParseBuffer        042 10803
#pragma  libcall ExpatBase XML_GetBuffer          048 0802
#pragma  libcall ExpatBase XML_SetStartElementHandler 04e 9802
#pragma  libcall ExpatBase XML_SetEndElementHandler 054 9802
#pragma  libcall ExpatBase XML_SetElementHandler  05a a9803
#pragma  libcall ExpatBase XML_SetCharacterDataHandler 060 9802
#pragma  libcall ExpatBase XML_SetProcessingInstructionHandler 066 9802
#pragma  libcall ExpatBase XML_SetCommentHandler  06c 9802
#pragma  libcall ExpatBase XML_SetStartCdataSectionHandler 072 9802
#pragma  libcall ExpatBase XML_SetEndCdataSectionHandler 078 9802
#pragma  libcall ExpatBase XML_SetCdataSectionHandler 07e a9803
#pragma  libcall ExpatBase XML_SetDefaultHandler  084 9802
#pragma  libcall ExpatBase XML_SetDefaultHandlerExpand 08a 9802
#pragma  libcall ExpatBase XML_SetExternalEntityRefHandler 090 9802
#pragma  libcall ExpatBase XML_SetExternalEntityRefHandlerArg 096 9802
#pragma  libcall ExpatBase XML_SetUnknownEncodingHandler 09c a9803
#pragma  libcall ExpatBase XML_SetStartNamespaceDeclHandler 0a2 9802
#pragma  libcall ExpatBase XML_SetEndNamespaceDeclHandler 0a8 9802
#pragma  libcall ExpatBase XML_SetNamespaceDeclHandler 0ae a9803
#pragma  libcall ExpatBase XML_SetXmlDeclHandler  0b4 9802
#pragma  libcall ExpatBase XML_SetStartDoctypeDeclHandler 0ba 9802
#pragma  libcall ExpatBase XML_SetEndDoctypeDeclHandler 0c0 9802
#pragma  libcall ExpatBase XML_SetDoctypeDeclHandler 0c6 a9803
#pragma  libcall ExpatBase XML_SetElementDeclHandler 0cc 9802
#pragma  libcall ExpatBase XML_SetAttlistDeclHandler 0d2 9802
#pragma  libcall ExpatBase XML_SetEntityDeclHandler 0d8 9802
#pragma  libcall ExpatBase XML_SetUnparsedEntityDeclHandler 0de 9802
#pragma  libcall ExpatBase XML_SetNotationDeclHandler 0e4 9802
#pragma  libcall ExpatBase XML_SetNotStandaloneHandler 0ea 9802
#pragma  libcall ExpatBase XML_GetErrorCode       0f0 801
#pragma  libcall ExpatBase XML_ErrorString        0f6 001
#pragma  libcall ExpatBase XML_GetCurrentByteIndex 0fc 801
#pragma  libcall ExpatBase XML_GetCurrentLineNumber 102 801
#pragma  libcall ExpatBase XML_GetCurrentColumnNumber 108 801
#pragma  libcall ExpatBase XML_GetCurrentByteCount 10e 801
#pragma  libcall ExpatBase XML_GetInputContext    114 a9803
#pragma  libcall ExpatBase XML_SetUserData        11a 9802
#pragma  libcall ExpatBase XML_DefaultCurrent     120 801
#pragma  libcall ExpatBase XML_UseParserAsHandlerArg 126 801
#pragma  libcall ExpatBase XML_SetBase            12c 9802
#pragma  libcall ExpatBase XML_GetBase            132 801
#pragma  libcall ExpatBase XML_GetSpecifiedAttributeCount 138 801
#pragma  libcall ExpatBase XML_GetIdAttributeIndex 13e 801
#pragma  libcall ExpatBase XML_SetEncoding        144 9802
#pragma  libcall ExpatBase XML_SetParamEntityParsing 14a 0802
#pragma  libcall ExpatBase XML_SetReturnNSTriplet 150 0802
#pragma  libcall ExpatBase XML_ExpatVersion       156 00
#pragma  libcall ExpatBase XML_ExpatVersionInfo   15c 00
#pragma  libcall ExpatBase XML_ParserReset        162 9802
#pragma  libcall ExpatBase XML_SetSkippedEntityHandler 168 9802
#pragma  libcall ExpatBase XML_UseForeignDTD      16e 0802
#pragma  libcall ExpatBase XML_GetFeatureList     174 00
#pragma  libcall ExpatBase XML_StopParser         17a 0802
#pragma  libcall ExpatBase XML_ResumeParser       180 801
#pragma  libcall ExpatBase XML_GetParsingStatus   186 9802
#pragma  libcall ExpatBase XML_FreeContentModel   18c 9802
#pragma  libcall ExpatBase XML_MemMalloc          192 0802
#pragma  libcall ExpatBase XML_MemRealloc         198 09803
#pragma  libcall ExpatBase XML_MemFree            19e 9802
#endif

#endif /* PRAGMAS_AEXPAT_PRAGMAS_H */
