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

/**
  An ACPI_PARSER array describing the ACPI MADT Table.
**/
STATIC CONST ACPI_PARSER  RhctParser[] = {
  PARSE_ACPI_HEADER (&AcpiHdrInfo),
  { L"Reserved",           4,  36, L"0x%x",     NULL,       NULL, NULL, NULL },
  { L"Time Base Frequency",8,  40, L"%d",       NULL,       NULL, NULL, NULL },
  { L"RHCT Node Number",   4,  48, L"%d",       NULL,       NULL, NULL, NULL },
  { L"RHCT Node Offset",   4,  52, L"0x%x",     NULL,       NULL, NULL, NULL }
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
  ParseAcpi (
             TRUE,
             0,
             "RHCT",
             Ptr,
             AcpiTableLength,
             PARSER_PARAMS (RhctParser)
             );
}
