/** @file
  RHCT table parser

  Copyright (c) 2024, Alibaba Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Reference(s):
    - RISC-V Hart Capabilities Table (RHCT) Table
      https://drive.google.com/file/d/1oMGPyOD58JaPgMl1pKasT-VKsIKia7zR/view
**/

#include <IndustryStandard/Acpi.h>
#include <Library/UefiLib.h>
#include "AcpiParser.h"
#include "AcpiTableParser.h"
#include "AcpiViewConfig.h"
#include "RhctParser.h"

// Local Variables
STATIC ACPI_DESCRIPTION_HEADER_INFO  AcpiHdrInfo;
STATIC CONST UINT16                   *RhctNodeType;
STATIC CONST UINT16                   *RhctNodeLength;
STATIC CONST UINT16                   *IsaStringLength;
STATIC CONST UINT16                   *Number;

/**
  An ACPI_PARSER array describing the ACPI RHCT Table.
**/
STATIC CONST ACPI_PARSER  RhctParser[] = {
  PARSE_ACPI_HEADER (&AcpiHdrInfo),
  { L"Reserved",           4,  36, L"0x%x",     NULL,       NULL, NULL, NULL },
  { L"Time Base Frequency",8,  40, L"%d",       NULL,       NULL, NULL, NULL },
  { L"RHCT Node Number",   4,  48, L"%d",       NULL,       NULL, NULL, NULL },
  { L"RHCT Node Offset",   4,  52, L"0x%x",     NULL,       NULL, NULL, NULL }
};

/**
  An ACPI_PARSER array describing the RHCT Node Header Structure.
**/
STATIC CONST ACPI_PARSER  RhctNodeHeaderParser[] = {
  { L"Type",        2,  0, L"%d", NULL, (VOID **)&RhctNodeType,   NULL, NULL },
  { L"Length",      2,  2, L"%d", NULL, (VOID **)&RhctNodeLength, NULL, NULL },
  { L"Revision",    2,  4, L"%d", NULL, NULL,                     NULL, NULL }
};

/**
  An ACPI_PARSER array describing the RHCT ISA String Node Structure.
**/
STATIC CONST ACPI_PARSER  RhctIsaStringNodeParser[] = {
  { L"Type",        2,  0, L"%d", NULL, (VOID **)&RhctNodeType,   NULL, NULL },
  { L"Length",      2,  2, L"%d", NULL, (VOID **)&RhctNodeLength, NULL, NULL },
  { L"Revision",    2,  4, L"%d", NULL, NULL,                     NULL, NULL },
  { L"ISA Length",  2,  6, L"%d", NULL, (VOID **)&IsaStringLength, NULL, NULL }
};

/**
  An ACPI_PARSER array describing the RHCT CMO Extension Node Structure.
**/
STATIC CONST ACPI_PARSER  RhctCmoExtensionNodeParser[] = {
  { L"Type",               2,  0, L"%d",   NULL, (VOID **)&RhctNodeType,   NULL, NULL },
  { L"Length",             2,  2, L"%d",   NULL, (VOID **)&RhctNodeLength, NULL, NULL },
  { L"Revision",           2,  4, L"%d",   NULL, NULL,                     NULL, NULL },
  { L"Reserved",           1,  6, L"0x%x", NULL, NULL,                     NULL, NULL },
  { L"CBOM Block Size",    1,  7, L"0x%x", NULL, NULL,                     NULL, NULL },
  { L"CBOP Block Size",    1,  8, L"0x%x", NULL, NULL,                     NULL, NULL },
  { L"CBOZ Block Size",    1,  9, L"0x%x", NULL, NULL,                     NULL, NULL }
};

/**
  An ACPI_PARSER array describing the RHCT MMU Node Structure.
**/
STATIC CONST ACPI_PARSER  RhctMmuNodeParser[] = {
  { L"Type",               2,  0, L"%d",   NULL, (VOID **)&RhctNodeType,   NULL, NULL },
  { L"Length",             2,  2, L"%d",   NULL, (VOID **)&RhctNodeLength, NULL, NULL },
  { L"Revision",           2,  4, L"%d",   NULL, NULL,                     NULL, NULL },
  { L"Reserved",           1,  6, L"0x%x", NULL, NULL,                     NULL, NULL },
  { L"MMU Type",           1,  7, L"0x%x", NULL, NULL,                     NULL, NULL }
};

/**
  An ACPI_PARSER array describing the RHCT HART Info Node Structure.
**/
STATIC CONST ACPI_PARSER  RhctHartInfoNodeParser[] = {
  { L"Type",                2,  0, L"%d", NULL, (VOID **)&RhctNodeType,   NULL, NULL },
  { L"Length",              2,  2, L"%d", NULL, (VOID **)&RhctNodeLength, NULL, NULL },
  { L"Revision",            2,  4, L"%d", NULL, NULL,                     NULL, NULL },
  { L"Offsets Number",      2,  6, L"%d", NULL, (VOID**)&Number,          NULL, NULL },
  { L"ACPI Processor UID",  4,  8, L"%d", NULL, NULL,                     NULL, NULL }
};

/**
  This function parses the ACPI RHCT table.

  This function also performs validation of the ACPI table fields.

  @param [in] Trace              If TRUE, trace the ACPI fields.
  @param [in] Ptr                Pointer to the start of the buffer.
  @param [in] AcpiTableLength    Length of the ACPI table.
  @param [in] AcpiTableRevision  Revision of the ACPI table.
**/
VOID
EFIAPI
ParseAcpiRhct (
  IN BOOLEAN  Trace,
  IN UINT8    *Ptr,
  IN UINT32   AcpiTableLength,
  IN UINT8    AcpiTableRevision
  )
{
  UINT32  Offset;
  UINT8   *RhctNodePtr;

  Offset = ParseAcpi (
             TRUE,
             0,
             "RHCT",
             Ptr,
             AcpiTableLength,
             PARSER_PARAMS (RhctParser)
             );

  RhctNodePtr = Ptr + Offset;

  while (Offset < AcpiTableLength) {
    // Parse RHCT Node header to obtain Type and Length.
    ParseAcpi (
      FALSE,
      0,
      NULL,
      RhctNodePtr,
      AcpiTableLength - Offset,
      PARSER_PARAMS (RhctNodeHeaderParser)
      );

    // Validate RHCT Node Structure length
    if ((*RhctNodeLength == 0) ||
        ((Offset + (*RhctNodeLength)) > AcpiTableLength))
    {
      IncrementErrorCount ();
      Print (
        L"ERROR: Invalid RHCT Node Structure length. " \
        L"Length = %d. Offset = %d. AcpiTableLength = %d.\n",
        *RhctNodeLength,
        Offset,
        AcpiTableLength
        );
      return;
    }

    switch (*RhctNodeType) {
      case EFI_ACPI_6_5_RHCT_NODE_TYPE_ISA_STRING_NODE:
      {
        ParseAcpi (
          TRUE,
          2,
          "ISA String Node",
          RhctNodePtr,
          *RhctNodeLength,
          PARSER_PARAMS (RhctIsaStringNodeParser)
          );

        if (*IsaStringLength != 0) {
          Print (L"    ISA String: %a\n", ((EFI_ACPI_6_5_RHCT_ISA_STRING_NODE_STRUCTURE *)RhctNodePtr)->ISAString);
        }
        break;
      }

      case EFI_ACPI_6_5_RHCT_NODE_TYPE_CMO_EXTENSION_NODE:
      {
        ParseAcpi (
          TRUE,
          2,
          "CMO Extension Node",
          RhctNodePtr,
          *RhctNodeLength,
          PARSER_PARAMS (RhctCmoExtensionNodeParser)
          );
        break;
      }


      case EFI_ACPI_6_5_RHCT_NODE_TYPE_MMU_NODE:
      {
        ParseAcpi (
          TRUE,
          2,
          "MMU Node",
          RhctNodePtr,
          *RhctNodeLength,
          PARSER_PARAMS (RhctMmuNodeParser)
          );
        break;
      }

      case EFI_ACPI_6_5_RHCT_NODE_TYPE_HART_INFO_NODE:
      {
        ParseAcpi (
          TRUE,
          2,
          "HART Info Node",
          RhctNodePtr,
          *RhctNodeLength,
          PARSER_PARAMS (RhctHartInfoNodeParser)
          );
        break;
      }
      default:
      {
        ParseAcpi (
          TRUE,
          2,
          "Unknown Node",
          RhctNodePtr,
          AcpiTableLength - Offset,
          PARSER_PARAMS (RhctNodeHeaderParser)
          );
        break;
      }

    }
    RhctNodePtr += *RhctNodeLength;
    Offset      += *RhctNodeLength;

  }
}
