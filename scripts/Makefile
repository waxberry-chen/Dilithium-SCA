ENV_PATH := /15T/Projects/Dilithium-SCA/scripts/env_setting/envs
ENV_NAME := dilithium-sca
INTERPRETER := $(ENV_PATH)/$(ENV_NAME)/bin/python3

ENV_FILE := python_env_conda.yml

# 默认目标
.DEFAULT_GOAL := help

# 帮助信息
help:
	@echo "------------------------------------------------------------------------------------------------"
	@echo "Usage: modify setting.json -> make dir -> copy power trace files to dir -> make process -> make cpa"
	@echo "------------------------------------------------------------------------------------------------"
	@echo "可用命令:"
	@echo "  make cpa       运行CPA攻击"
	@echo "  make process   处理功耗迹曲线"
	@echo "  make dir       自动生成数据目录"
	@echo "  make tvla      计算所给曲线的TVLA检验统计量"
	@echo "  make cor_curve 绘制 相关系数-功耗迹 曲线"
	@echo "------------------------------------------------------------------------------------------------"
	@echo "Tips: 修改配置文件 setting.json 中的 KEY_TRUE、FILE_NUM、DIR_TAG 等属性后 make dir 创建存放功耗迹的目录 "
	@echo "      修改配置文件 setting.json 中的 REMARK 属性在 <结果目录文件夹名> 记录此次修改, 对 相关系数-功耗迹 曲线，\
	会记录在生成的图片文件名中"



cpa: DilithiumCPA.py CpaAttack.py
	@$(INTERPRETER) $<

process: DilithiumTraces.py TraceProcess.py
	@$(INTERPRETER) $<

dir: DilithiumTraces.py TraceProcess.py 
	@$(INTERPRETER) $< -d

tvla: tvla.py
	@$(INTERPRETER) $<

cor_curve: GetCpaTraceNum.py
	@$(INTERPRETER) $<