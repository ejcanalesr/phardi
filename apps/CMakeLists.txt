ADD_EXECUTABLE(phardi phardi.cpp)
TARGET_LINK_LIBRARIES(phardi 
	${EXTRA_LIBS}
	${ArrayFire_Unified_LIBRARIES}
	${ArrayFire_CUDA}
	${Glue}  ${ITK_LIBRARIES} ${VTK_LIBRARIES}
        ${BLAS_LIBRARIES}  
        ${LAPACK_LIBRARIES}
        ${ARMADILLO_LIBRARIES}
        ${MKL_LIBRARIES}
)

