#!/bin/bash
#SBATCH --job-name=Astra
#SBATCH --partition=normal
#SBATCH -A yebohong
#SBATCH -N 1
#SBATCH -n 4
#SBATCH --gres=gpu:a6000:1
#SBATCH -t 24:00:00
#SBATCH --ntasks-per-node=4
#SBATCH --mail-type=end,fail
#SBATCH --mail-user=1871546194@qq.com
#SBATCH --output=/mnt/kunlun/users/yebohong/a6000_log/%j.out
#SBATCH --error=/mnt/kunlun/users/yebohong/a6000_log/%j.err

echo "Job started at: $(date)"
echo "Running on node: $(hostname)"
echo "Current directory: $(pwd)"

# 1. 加载软件环境
conda activate tigre


echo "Job finished at: $(date)"