#!/bin/bash
#SBATCH --job-name=geant4
#SBATCH --partition=normal
#SBATCH -A yebohong
#SBATCH -N 1
#SBATCH -n 50
#SBATCH -t 48:00:00
#SBATCH -w n13
#SBATCH --ntasks-per-node=50
#SBATCH --mail-type=end,fail
#SBATCH --mail-user=1871546194@qq.com
#SBATCH --output=/mnt/kunlun/users/yebohong/Brain_threeLoop/three_loop/log/%j.out
#SBATCH --error=/mnt/kunlun/users/yebohong/Brain_threeLoop/three_loop/log/%j.err

echo "Job started at: $(date)"
echo "Running on node: $(hostname)"
echo "Current directory: $(pwd)"

# 1. 加载软件环境
module load cmake
module load geant4
# 2. 进入程序目录
cd /mnt/kunlun/users/yebohong/Brain_threeLoop/three_loop

# 3. 如果还没编译，就编译

cd build
cmake ..
make -j50

# 4. 运行 Geant4 程序
./GammaTherapy run2.mac

echo "Job finished at: $(date)"
