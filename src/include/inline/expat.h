#ifndef _INLINE_EXPAT_H
#define _INLINE_EXPAT_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef EXPAT_BASE_NAME
#define EXPAT_BASE_NAME ExpatBase
#endif

#define XML_ParserCreate(encodingName) \
  LP1(0x1e, XML_Parser, XML_ParserCreate, const XML_Char *, encodingName, a0, \
  , EXPAT_BASE_NAME)

#define XML_ParserCreateNS(encodingName, nsSep) \
  LP2(0x24, XML_Parser, XML_ParserCreateNS, const XML_Char *, encodingName, a0, XML_Char, nsSep, d0, \
  , EXPAT_BASE_NAME)

#define XML_ParserCreate_MM(encoding, memsuite, namespaceSeparator) \
  LP3(0x2a, XML_Parser, XML_ParserCreate_MM, const XML_Char *, encoding, a0, const XML_Memory_Handling_Suite *, memsuite, a1, const XML_Char *, namespaceSeparator, a2, \
  , EXPAT_BASE_NAME)

#define XML_ExternalEntityParserCreate(parser, context, encoding) \
  LP3(0x30, XML_Parser, XML_ExternalEntityParserCreate, XML_Parser, parser, a0, const XML_Char *, context, a1, const XML_Char *, encoding, a2, \
  , EXPAT_BASE_NAME)

#define XML_ParserFree(parser) \
  LP1NR(0x36, XML_ParserFree, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_Parse(parser, s, len, isFinal) \
  LP4(0x3c, int, XML_Parse, XML_Parser, parser, a0, const char *, s, a1, int, len, d0, int, isFinal, d1, \
  , EXPAT_BASE_NAME)

#define XML_ParseBuffer(parser, len, isFinal) \
  LP3(0x42, int, XML_ParseBuffer, XML_Parser, parser, a0, int, len, d0, int, isFinal, d1, \
  , EXPAT_BASE_NAME)

#define XML_GetBuffer(parser, len) \
  LP2(0x48, void *, XML_GetBuffer, XML_Parser, parser, a0, int, len, d0, \
  , EXPAT_BASE_NAME)

#define XML_SetStartElementHandler(parser, start) \
  LP2NR(0x4e, XML_SetStartElementHandler, XML_Parser, parser, a0, XML_StartElementHandler, start, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetEndElementHandler(parser, end) \
  LP2NR(0x54, XML_SetEndElementHandler, XML_Parser, parser, a0, XML_EndElementHandler, end, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetElementHandler(parser, start, end) \
  LP3NR(0x5a, XML_SetElementHandler, XML_Parser, parser, a0, XML_StartElementHandler, start, a1, XML_EndElementHandler, end, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetCharacterDataHandler(parser, handler) \
  LP2NR(0x60, XML_SetCharacterDataHandler, XML_Parser, parser, a0, XML_CharacterDataHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetProcessingInstructionHandler(parser, handler) \
  LP2NR(0x66, XML_SetProcessingInstructionHandler, XML_Parser, parser, a0, XML_ProcessingInstructionHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetCommentHandler(parser, handler) \
  LP2NR(0x6c, XML_SetCommentHandler, XML_Parser, parser, a0, XML_CommentHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetStartCdataSectionHandler(parser, start) \
  LP2NR(0x72, XML_SetStartCdataSectionHandler, XML_Parser, parser, a0, XML_StartCdataSectionHandler, start, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetEndCdataSectionHandler(parser, end) \
  LP2NR(0x78, XML_SetEndCdataSectionHandler, XML_Parser, parser, a0, XML_EndCdataSectionHandler, end, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetCdataSectionHandler(parser, start, end) \
  LP3NR(0x7e, XML_SetCdataSectionHandler, XML_Parser, parser, a0, XML_StartCdataSectionHandler, start, a1, XML_EndCdataSectionHandler, end, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetDefaultHandler(parser, handler) \
  LP2NR(0x84, XML_SetDefaultHandler, XML_Parser, parser, a0, XML_DefaultHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetDefaultHandlerExpand(parser, handler) \
  LP2NR(0x8a, XML_SetDefaultHandlerExpand, XML_Parser, parser, a0, XML_DefaultHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetExternalEntityRefHandler(parser, handler) \
  LP2NR(0x90, XML_SetExternalEntityRefHandler, XML_Parser, parser, a0, XML_ExternalEntityRefHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetExternalEntityRefHandlerArg(parser, arg) \
  LP2NR(0x96, XML_SetExternalEntityRefHandlerArg, XML_Parser, parser, a0, void *, arg, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetUnknownEncodingHandler(parser, handler, data) \
  LP3NR(0x9c, XML_SetUnknownEncodingHandler, XML_Parser, parser, a0, XML_UnknownEncodingHandler, handler, a1, void *, data, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetStartNamespaceDeclHandler(parser, start) \
  LP2NR(0xa2, XML_SetStartNamespaceDeclHandler, XML_Parser, parser, a0, XML_StartNamespaceDeclHandler, start, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetEndNamespaceDeclHandler(parser, handler) \
  LP2NR(0xa8, XML_SetEndNamespaceDeclHandler, XML_Parser, parser, a0, XML_EndNamespaceDeclHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetNamespaceDeclHandler(parser, start, end) \
  LP3NR(0xae, XML_SetNamespaceDeclHandler, XML_Parser, parser, a0, XML_StartNamespaceDeclHandler, start, a1, XML_EndNamespaceDeclHandler, end, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetXmlDeclHandler(parser, handler) \
  LP2NR(0xb4, XML_SetXmlDeclHandler, XML_Parser, parser, a0, XML_XmlDeclHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetStartDoctypeDeclHandler(parser, start) \
  LP2NR(0xba, XML_SetStartDoctypeDeclHandler, XML_Parser, parser, a0, XML_StartDoctypeDeclHandler, start, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetEndDoctypeDeclHandler(parser, end) \
  LP2NR(0xc0, XML_SetEndDoctypeDeclHandler, XML_Parser, parser, a0, XML_EndDoctypeDeclHandler, end, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetDoctypeDeclHandler(parser, start, end) \
  LP3NR(0xc6, XML_SetDoctypeDeclHandler, XML_Parser, parser, a0, XML_StartDoctypeDeclHandler, start, a1, XML_EndDoctypeDeclHandler, end, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetElementDeclHandler(parser, eldecl) \
  LP2NR(0xcc, XML_SetElementDeclHandler, XML_Parser, parser, a0, XML_ElementDeclHandler, eldecl, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetAttlistDeclHandler(parser, attdecl) \
  LP2NR(0xd2, XML_SetAttlistDeclHandler, XML_Parser, parser, a0, XML_AttlistDeclHandler, attdecl, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetEntityDeclHandler(parser, handler) \
  LP2NR(0xd8, XML_SetEntityDeclHandler, XML_Parser, parser, a0, XML_EntityDeclHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetUnparsedEntityDeclHandler(parser, handler) \
  LP2NR(0xde, XML_SetUnparsedEntityDeclHandler, XML_Parser, parser, a0, XML_UnparsedEntityDeclHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetNotationDeclHandler(parser, handler) \
  LP2NR(0xe4, XML_SetNotationDeclHandler, XML_Parser, parser, a0, XML_NotationDeclHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetNotStandaloneHandler(parser, handler) \
  LP2NR(0xea, XML_SetNotStandaloneHandler, XML_Parser, parser, a0, XML_NotStandaloneHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_GetErrorCode(parser) \
  LP1(0xf0, enum XML_Error, XML_GetErrorCode, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_ErrorString(code) \
  LP1(0xf6, const XML_LChar *, XML_ErrorString, int, code, d0, \
  , EXPAT_BASE_NAME)

#define XML_GetCurrentByteIndex(parser) \
  LP1(0xfc, long, XML_GetCurrentByteIndex, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetCurrentLineNumber(parser) \
  LP1(0x102, int, XML_GetCurrentLineNumber, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetCurrentColumnNumber(parser) \
  LP1(0x108, int, XML_GetCurrentColumnNumber, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetCurrentByteCount(parser) \
  LP1(0x10e, int, XML_GetCurrentByteCount, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetInputContext(parser, offset, size) \
  LP3(0x114, const char *, XML_GetInputContext, XML_Parser, parser, a0, int *, offset, a1, int *, size, a2, \
  , EXPAT_BASE_NAME)

#define XML_SetUserData(parser, p) \
  LP2NR(0x11a, XML_SetUserData, XML_Parser, parser, a0, void *, p, a1, \
  , EXPAT_BASE_NAME)

#define XML_DefaultCurrent(parser) \
  LP1NR(0x120, XML_DefaultCurrent, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_UseParserAsHandlerArg(parser) \
  LP1NR(0x126, XML_UseParserAsHandlerArg, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_SetBase(parser, p) \
  LP2(0x12c, int, XML_SetBase, XML_Parser, parser, a0, const XML_Char *, p, a1, \
  , EXPAT_BASE_NAME)

#define XML_GetBase(parser) \
  LP1(0x132, const XML_Char *, XML_GetBase, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetSpecifiedAttributeCount(parser) \
  LP1(0x138, int, XML_GetSpecifiedAttributeCount, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_GetIdAttributeIndex(parser) \
  LP1(0x13e, int, XML_GetIdAttributeIndex, XML_Parser, parser, a0, \
  , EXPAT_BASE_NAME)

#define XML_SetEncoding(parser, encoding) \
  LP2(0x144, int, XML_SetEncoding, XML_Parser, parser, a0, const XML_Char *, encoding, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetParamEntityParsing(parser, parsing) \
  LP2(0x14a, int, XML_SetParamEntityParsing, XML_Parser, parser, a0, enum XML_ParamEntityParsing, parsing, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetReturnNSTriplet(parser, do_nst) \
  LP2NR(0x150, XML_SetReturnNSTriplet, XML_Parser, parser, a0, int, do_nst, d0, \
  , EXPAT_BASE_NAME)

#define XML_ExpatVersion() \
  LP0(0x156, const XML_LChar *, XML_ExpatVersion, \
  , EXPAT_BASE_NAME)

#define XML_ExpatVersionInfo() \
  LP0(0x15c, XML_Expat_Version, XML_ExpatVersionInfo, \
  , EXPAT_BASE_NAME)

#define XML_ParserReset(parser, encoding) \
  LP2(0x162, int, XML_ParserReset, XML_Parser, parser, a0, const XML_Char *, encoding, a1, \
  , EXPAT_BASE_NAME)

#define XML_SetSkippedEntityHandler(parser, handler) \
  LP2NR(0x168, XML_SetSkippedEntityHandler, XML_Parser, parser, a0, XML_SkippedEntityHandler, handler, a1, \
  , EXPAT_BASE_NAME)

#define XML_UseForeignDTD(parser, useDTD) \
  LP2(0x16e, enum XML_Error, XML_UseForeignDTD, XML_Parser, parser, a0, XML_Bool, useDTD, d0, \
  , EXPAT_BASE_NAME)

#define XML_GetFeatureList() \
  LP0(0x174, const XML_Feature *, XML_GetFeatureList, \
  , EXPAT_BASE_NAME)

#endif /*  _INLINE_EXPAT_H  */
