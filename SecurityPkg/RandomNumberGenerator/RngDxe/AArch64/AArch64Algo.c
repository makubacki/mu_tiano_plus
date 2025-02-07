/** @file
  Aarch64 specific code.

  Copyright (c) 2022, Arm Limited. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ArmTrngLib.h>

#include "RngDxeInternals.h"

// Maximum number of Rng algorithms.
#define RNG_AVAILABLE_ALGO_MAX  2

/** Allocate and initialize mAvailableAlgoArray with the available
    Rng algorithms. Also update mAvailableAlgoArrayCount.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
EFI_STATUS
EFIAPI
GetAvailableAlgorithms (
  VOID
  )
{
  UINT64  DummyRand;
  UINT16  MajorRevision;
  UINT16  MinorRevision;

  // Rng algorithms 2 times, one for the allocation, one to populate.
  mAvailableAlgoArray = AllocateZeroPool (RNG_AVAILABLE_ALGO_MAX * sizeof (EFI_RNG_ALGORITHM));
  if (mAvailableAlgoArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Check RngGetBytes() before advertising PcdCpuRngSupportedAlgorithm.
  if (!EFI_ERROR (RngGetBytes (sizeof (DummyRand), (UINT8 *)&DummyRand))) {
    CopyMem (
      &mAvailableAlgoArray[mAvailableAlgoArrayCount],
      PcdGetPtr (PcdCpuRngSupportedAlgorithm),
      sizeof (EFI_RNG_ALGORITHM)
      );
    mAvailableAlgoArrayCount++;

    if (IsZeroGuid (PcdGetPtr (PcdCpuRngSupportedAlgorithm))) {
      DEBUG_CODE_BEGIN ();
      DEBUG ((
        DEBUG_WARN,
        "PcdCpuRngSupportedAlgorithm should be a non-zero GUID\n"
        ));

      DEBUG_CODE_END ();
      mAvailableAlgoArrayCount--;
    }
  }

  // Raw algorithm (Trng)
  if (!EFI_ERROR (GetArmTrngVersion (&MajorRevision, &MinorRevision))) {
    CopyMem (
      &mAvailableAlgoArray[mAvailableAlgoArrayCount],
      &gEfiRngAlgorithmRaw,
      sizeof (EFI_RNG_ALGORITHM)
      );
    mAvailableAlgoArrayCount++;
  }

  return EFI_SUCCESS;
}
