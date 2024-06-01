set(CMS_QPCPP_SRC_DIR ${CMS_QPCPP_TOP_DIR}/src)
set(CMS_QPCPP_INCLUDE_DIR ${CMS_QPCPP_TOP_DIR}/include)
set(CMS_QPCPP_QF_SRC_DIR ${CMS_QPCPP_SRC_DIR}/qf)

set(CMS_QPCPP_QF_SRCS
	${CMS_QPCPP_QF_SRC_DIR}/qep_hsm.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qep_msm.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_act.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_actq.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_defer.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_dyn.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_mem.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_ps.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_qact.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_qeq.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_qmact.cpp
	${CMS_QPCPP_QF_SRC_DIR}/qf_time.cpp)
