/*************************************************************************
 * Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/
#ifndef TEST_REDUCE_HPP
#define TEST_REDUCE_HPP

#include "CorrectnessTest.hpp"

namespace CorrectnessTests
{
    class ReduceCorrectnessTest : public CorrectnessTest
    {
    public:
        static void ComputeExpectedResults(Dataset& dataset, ncclRedOp_t const op, int const root)
        {
            // Copy all inputs to expected arrays temporarily to perform reduction on host
            for (int i = 0; i < dataset.numDevices; i++)
                HIP_CALL(hipMemcpy(dataset.expected[i], dataset.inputs[i],
                                   dataset.NumBytes(), hipMemcpyDeviceToHost));

            // Allocate temporary host array to accumulate results
            int8_t*   resultI1 = (int8_t   *)malloc(dataset.NumBytes());
            uint8_t*  resultU1 = (uint8_t  *)resultI1;
            int32_t*  resultI4 = (int32_t  *)resultI1;
            uint32_t* resultU4 = (uint32_t *)resultI1;
            int64_t*  resultI8 = (int64_t  *)resultI1;
            uint64_t* resultU8 = (uint64_t *)resultI1;
            float*    resultF4 = (float    *)resultI1;
            double*   resultF8 = (double   *)resultI1;
            rccl_bfloat16* resultB2 = (rccl_bfloat16 *)resultI1;

            // Initialize the result with the first device's array
            memcpy(resultI1, dataset.expected[0], dataset.NumBytes());
            ncclRedOp_t red_op = ((op == ncclAvg) ? ncclSum : op);

            // Perform reduction on the other device arrays
            for (int i = 1; i < dataset.numDevices; i++)
            {
                int8_t*   arrayI1 = (int8_t   *)dataset.expected[i];
                uint8_t*  arrayU1 = (uint8_t  *)arrayI1;
                int32_t*  arrayI4 = (int32_t  *)arrayI1;
                uint32_t* arrayU4 = (uint32_t *)arrayI1;
                int64_t*  arrayI8 = (int64_t  *)arrayI1;
                uint64_t* arrayU8 = (uint64_t *)arrayI1;
                float*    arrayF4 = (float    *)arrayI1;
                double*   arrayF8 = (double   *)arrayI1;
                rccl_bfloat16* arrayB2 = (rccl_bfloat16 *)arrayI1;

                for (int j = 0; j < dataset.numElements; j++)
                {
                    switch (dataset.dataType)
                    {
                    case ncclInt8:    resultI1[j] = ReduceOp(red_op, resultI1[j], arrayI1[j]); break;
                    case ncclUint8:   resultU1[j] = ReduceOp(red_op, resultU1[j], arrayU1[j]); break;
                    case ncclInt32:   resultI4[j] = ReduceOp(red_op, resultI4[j], arrayI4[j]); break;
                    case ncclUint32:  resultU4[j] = ReduceOp(red_op, resultU4[j], arrayU4[j]); break;
                    case ncclInt64:   resultI8[j] = ReduceOp(red_op, resultI8[j], arrayI8[j]); break;
                    case ncclUint64:  resultU8[j] = ReduceOp(red_op, resultU8[j], arrayU8[j]); break;
                    case ncclFloat32: resultF4[j] = ReduceOp(red_op, resultF4[j], arrayF4[j]); break;
                    case ncclFloat64: resultF8[j] = ReduceOp(red_op, resultF8[j], arrayF8[j]); break;
                    case ncclBfloat16: resultB2[j] = ReduceOp(red_op, resultB2[j], arrayB2[j]); break;
                    default:
                        fprintf(stderr, "[ERROR] Unsupported datatype\n");
                        exit(0);
                    }
                }
            }

            if (op == ncclAvg)
                Average(dataset, resultI1);

            // Copy results into expected arrays
            for (int i = 0; i < dataset.numDevices; i++)
            {
                if (i == root)
                    memcpy(dataset.expected[root], resultI1, dataset.NumBytes());
                else
                    HIP_CALL(hipMemcpy(dataset.expected[i], dataset.outputs[i], dataset.NumBytes(), hipMemcpyDeviceToHost));
            }
            free(resultI1);
        }
    };
}

#endif
