/*
 * vrg.h
 *
 * Created: 03.12.2024 15:42:40
 *  Author: https://dev.to/rdentato/default-parameters-in-c-4kl1
 *
 */ 


#ifndef VRG_H_
#define VRG_H_

#define VRG_CNT(vrg0,vrg1,vrg2,vrg3,vrg4,vrg5,vrg6,vrg7,vrg8,vrgN, ...) vrgN
#define VRG_ARGN(...)  VRG_CNT(_, ## __VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define VRG_CAT_(x,y)  x ## y
#define VRG_CAT(x,y)   VRG_CAT_(x,y)

#define VRG(vrg_f,...) VRG_CAT(vrg_f, VRG_ARGN(__VA_ARGS__))(__VA_ARGS__)



#endif /* VRG_H_ */