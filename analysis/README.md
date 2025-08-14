# TB2025 Analysis Package - Refactored Version

이 패키지는 TB2025 빔테스트 데이터 분석을 위한 리팩토링된 코드입니다.

## 주요 개선사항

### 1. 설정 파일 기반 구성 (config.txt)
- 모든 분석 파라미터를 `config.txt` 파일에서 중앙 관리
- 적분 범위, 컷 값, 보정 상수 등을 코드 수정 없이 변경 가능

### 2. 계산된 보정 상수 활용
- `calc_calib_refactored.cc`에서 계산된 보정 상수를 자동으로 활용
- Config 포맷, JSON 포맷, CSV 포맷으로 보정 상수 저장
- 계산된 보정 상수가 없으면 기본값 자동 사용

### 3. 🆕 설정 가능한 DRC 에너지 해상도 분석
- **새로운 기능**: `calib_DRC_configurable.cc`
- 중심 모듈을 자유롭게 선택 가능 (M5T3, M8T2, M5T2 등)
- 전체 DRC 또는 3x3 영역 선택 가능
- config.txt에서 간단한 설정 변경으로 다양한 구성 테스트

### 4. 🆕 물리적 레이아웃 기반 타워 선택
- **새로운 기능**: `module_position.txt` 파일 기반 지능형 타워 선택
- 실제 검출기의 물리적 배치를 정확히 반영
- 경계 타워 자동 처리 (M6T2 같은 경우 자동으로 6개만 선택)
- 다양한 크기 지원: 3x3, 5x5, 7x7, 9x9
- 존재하지 않는 타워 조합 자동 제외

### 5. 완전 자동화된 분석 체인
- **새로운 기능**: `run_analysis_chain.sh` 배치 스크립트
- 한 번의 명령으로 전체 분석 과정 자동 실행
- 오류 검사 및 진행 상황 표시

### 6. 히스토그램 관리 시스템
- `HistogramManager` 클래스로 히스토그램 생성, Fill, Write를 체계적으로 관리
- 루프를 사용한 자동화로 수동 나열 방식 제거
- 메모리 관리 자동화

### 7. 모듈화된 구조
- 기능별로 분리된 헤더 파일들
- 재사용 가능한 코드 구조

## 파일 구조

```
analysis/Guk/
├── config.txt                      # 설정 파일
├── header/
│   ├── config.h                    # 설정 관리 클래스
│   ├── HistogramManager.h          # 히스토그램 관리 클래스
│   ├── function.h                  # 공통 함수들
│   └── module_position.txt         # 🆕 DRC 물리적 레이아웃 정의
├── calib_DRC_configurable.cc       # 🆕 설정 가능한 DRC 에너지 해상도 분석
├── draw_DWC_refactored.cc          # DWC 분석 (리팩토링)
├── avgTimeStruc_refactored.cc      # 평균 시간 구조 분석 (리팩토링)
├── calc_calib_refactored.cc        # 보정 상수 계산 (리팩토링)
├── calib_singleModule_refactored.cc # 단일 모듈 보정 (리팩토링)
├── compile.sh                      # 컴파일 스크립트
├── run_analysis_chain.sh           # 🆕 자동 분석 체인 스크립트
└── README.md                       # 이 파일
```

## 🆕 설정 가능한 DRC 분석 사용법

### 기본 사용법:
```bash
# 컴파일
./compile.sh

# 기본 설정으로 실행 (M8T2 중심, 전체 DRC)
./calib_DRC_configurable 9999 100000
```

### 다양한 설정 예시:

#### 1. M5T3 중심, 3x3 영역 분석:
config.txt에서:
```
moduleCenter=M5T3
size=3x3
```
**새로운 기능**: `module_position.txt` 파일을 참고해서 실제 물리적 배치에 맞게 타워들을 선택합니다.
M5T3를 중심으로 하는 3x3 영역은 다음과 같습니다:
```
M4T2  M5T1  M5T2
M4T4  M5T3  M5T4  
M7T2  M8T1  M8T2
```

#### 2. M6T2 중심, 3x3 영역 분석 (경계 케이스):
config.txt에서:
```
moduleCenter=M6T2
size=3x3
```
우측 경계에 있는 M6T2를 중심으로 하면 가능한 타워들만 선택됩니다:
```
M3T3  M3T4
M6T1  M6T2
M6T3  M6T4
```
총 6개 타워가 선택됩니다.

#### 3. M5T3 중심, 5x5 영역 분석:
config.txt에서:
```
moduleCenter=M5T3
size=5x5
```
더 큰 영역을 분석하고 싶을 때 사용합니다.

#### 4. M8T2 중심, 전체 DRC 분석:
config.txt에서:
```
moduleCenter=M8T2
size=full
```

### 지원하는 크기 옵션:
- `full`: 전체 DRC (M1-M9, 모든 타워)
- `3x3`: 중심 타워 기준 3x3 영역
- `5x5`: 중심 타워 기준 5x5 영역  
- `7x7`: 중심 타워 기준 7x7 영역
- `9x9`: 중심 타워 기준 9x9 영역 (거의 전체)

### 출력 파일:
- `output_DRC_{moduleCenter}_{size}_{run_number}.root`
- 예시: `output_DRC_M5T3_3x3_9999.root`

## 🆕 물리적 레이아웃 기반 타워 선택

### module_position.txt 파일
DRC의 실제 물리적 배치를 정의하는 파일입니다:

```
#upstream
M1T1 M1T2 M2T1 M2T2 M3T1 M3T2
M1T3 M1T4 M2T3 M2T4 M3T3 M3T4
M4T1 M4T2 M5T1 M5T2 M6T1 M6T2
M4T3 M4T4 M5T3 M5T4 M6T3 M6T4
M7T1 M7T2 M8T1 M8T2 M9T1 M9T2
M7T3 M7T4 M8T3 M8T4 M9T3 M9T4
```

이 파일을 통해:
- **정확한 물리적 배치**: 실제 검출기의 기하학적 구조를 반영
- **자동 경계 처리**: 가장자리 타워들은 자동으로 가능한 타워들만 선택
- **유연한 크기 선택**: 3x3, 5x5, 7x7 등 다양한 크기 지원
- **에러 방지**: 존재하지 않는 타워 조합을 자동으로 제외

### 작동 원리:
1. 중심 타워(예: M5T3)의 grid 상 위치를 찾음
2. 중심을 기준으로 NxN 영역의 타워들을 선택
3. Grid 경계를 벗어나는 위치는 자동으로 제외
4. 실제로 존재하는 타워들만 최종 선택

### 테스트 예시:
```bash
# M5T3 중심, 3x3 영역으로 테스트
# config.txt에서 moduleCenter=M5T3, size=3x3로 설정
./calib_DRC_configurable 9999 1000

# 출력에서 다음과 같은 메시지 확인:
# Module position layout loaded: 6x6 grid
# Center M5T3 found at grid position (2,3)
# Selecting 3x3 region around center:
#   M4T2  M5T1  M5T2  M4T4  M5T3  M5T4  M7T2  M8T1  M8T2
# Total selected towers: 9

# M6T2 경계 케이스 테스트
# config.txt에서 moduleCenter=M6T2, size=3x3로 설정
./calib_DRC_configurable 9999 1000

# 출력에서 다음과 같은 메시지 확인:
# Center M6T2 found at grid position (2,5)
# Selecting 3x3 region around center:
#   M3T3  M3T4  M6T1  M6T2  M6T3  M6T4
# Total selected towers: 6 (경계라서 6개만 선택됨)
```

## 새로운 분석 워크플로우

### 🚀 원클릭 자동 분석 (권장):

```bash
# 전체 분석 체인을 한 번에 실행
./run_analysis_chain.sh 1234 10000 8

# DWC 분석 스킵하고 실행 (기존 DWC 파일 사용)
./run_analysis_chain.sh 1234 10000 8 skip
```

### 🔧 수동 단계별 분석:

```bash
# 1단계: DWC 빔 위치 분석
./draw_DWC_refactored 1234 10000

# 2단계: 평균 시간 구조 분석 (필요시)
./avgTimeStruc_refactored 1234 10000

# 3단계: 초기 보정 (기본 보정 상수 사용)
./calib_singleModule_refactored 1234 10000 8

# 4단계: 정밀 보정 상수 계산
./calc_calib_refactored 1234 8

# 5단계: 최종 분석 (계산된 보정 상수 자동 사용)
./calib_singleModule_refactored 1234 10000 8
```

**자동 보정 상수 적용**: 5단계에서 프로그램이 자동으로 `./Calc_calib/Run_1234_Module_8_calib.txt` 파일을 찾아서 사용합니다.

## 설정 파일 (config.txt)

분석에 필요한 모든 파라미터를 설정할 수 있습니다:

### 적분 범위 설정
```
moduleFirst=200      # 모듈 적분 시작 bin
moduleLast=400       # 모듈 적분 끝 bin
cc1PeakFirst=600     # CC1 피크 검색 시작 bin
cc1PeakLast=850      # CC1 피크 검색 끝 bin
# ... (기타 detector 범위들)
```

### 컷 값 설정
```
cutCC1=60.0          # CC1 PID 컷
cutCC2=100.0         # CC2 PID 컷
cutPS=1000.0         # Preshower 컷
cutMC=35.0           # Muon counter 컷
cutDWC=2.0           # DWC 기하학적 컷
```

### DWC 보정 상수
```
dwc1HorizontalSlope=0.1830
dwc1HorizontalOffset=-0.04539
dwc1VerticalSlope=-0.1798
dwc1VerticalOffset=0.06994
# ... (DWC2 상수들)
```

### 기본 모듈 보정 상수 (fallback용)
```
calib_T1_C=0.0008399
calib_T1_S=0.000559943
# ... (각 타워별 상수들)
```

## 사용법

### 1. 설정 파일 수정
원하는 분석 조건에 맞게 `config.txt` 파일의 파라미터들을 수정합니다.

### 2. 컴파일
```bash
./compile_refactored.sh
```

### 3. 분석 실행

#### 🚀 자동 분석 (권장)
```bash
# 완전 자동 분석
./run_analysis_chain.sh [RunNumber] [MaxEvent] [ModuleNumber]

# 예시: Run 1234, 10000 events, Module 8
./run_analysis_chain.sh 1234 10000 8

# DWC 분석 스킵 (기존 DWC 파일 사용)
./run_analysis_chain.sh 1234 10000 8 skip
```

#### 🔧 수동 분석
```bash
# DWC 분석
./draw_DWC_refactored [RunNumber] [MaxEvent]

# 단일 모듈 보정 (자동으로 계산된 보정 상수 사용)
./calib_singleModule_refactored [RunNumber] [MaxEvent] [ModuleNumber]

# 보정 상수 계산
./calc_calib_refactored [RunNumber] [ModuleNumber]

# 특정 보정 상수 파일 사용
./calib_singleModule_refactored 1234 10000 8 ./Calc_calib/Run_1234_Module_8_calib.txt
```

## 보정 상수 파일 포맷

`calc_calib_refactored.cc`는 세 가지 포맷으로 보정 상수를 저장합니다:

### 1. Config 포맷 (.txt) - 권장
```
# Calibration Constants for Module 8
# Generated from Run 1234
# Format: parameter=value (no spaces around =)
# Normalized to 60 GeV beam energy

calib_M8_T1_C=0.001234
calib_M8_T1_S=0.000567
calib_M8_T2_C=0.001345
calib_M8_T2_S=0.000678
...
```

### 2. JSON 포맷 (.json) - 고급 사용
```json
{
  "metadata": {
    "run_number": "1234",
    "module_number": "8",
    "beam_energy": "60 GeV",
    "generated_date": "Dec 10 2024 15:30:45"
  },
  "calibration_constants": {
    "calib_M8_T1_C": 0.001234,
    "calib_M8_T1_S": 0.000567,
    ...
  }
}
```

### 3. CSV 포맷 (.csv) - 호환성
```
calib_M8_T1_C 0.001234
calib_M8_T1_S 0.000567
...
```

## 주요 클래스 설명

### Config 클래스
- 설정 파일 읽기 및 관리
- 기본값 설정 및 파일 파싱
- **새로운 기능**: 계산된 보정 상수 파일 로딩
- 모듈별 보정 상수 추출

### HistogramManager 클래스
- 히스토그램 생성, 관리, 저장을 자동화
- 분석 타입별 히스토그램 초기화 함수
- 메모리 관리 자동화

## 자동 보정 상수 적용 로직

1. **자동 파일 검색**: `./Calc_calib/Run_[RunNumber]_Module_[ModuleNumber]_calib.txt`
2. **파일 존재 확인**: 파일이 있으면 자동으로 로드
3. **Fallback**: 파일이 없으면 config.txt의 기본값 사용
4. **사용자 지정**: 명령행으로 특정 파일 지정 가능

## 출력 파일 구분

분석 결과 파일명에 사용된 보정 상수 정보가 포함됩니다:
- `Calib_Run_1234_calcCalib.root`: 계산된 보정 상수 사용
- `Calib_Run_1234_defaultCalib.root`: 기본 보정 상수 사용

## 장점

1. **🚀 완전 자동화**: 보정 상수 계산 → 자동 적용
2. **⚡ 원클릭 분석**: 전체 분석 체인을 한 번에 실행
3. **🔧 유지보수성 향상**: 파라미터 변경 시 코드 재컴파일 불필요
4. **❌ 오류 감소**: 수동 히스토그램 관리에서 발생하는 실수 방지
5. **📈 확장성**: 새로운 detector나 분석 추가 용이
6. **🎯 일관성**: 모든 코드에서 동일한 설정 값 사용
7. **⚡ 효율성**: 루프 기반 자동화로 코드 간소화
8. **👁️ 투명성**: 어떤 보정 상수가 사용되었는지 명확히 표시

## 기존 코드와의 호환성

기존 코드들(`draw_DWC.cc`, `avgTimeStruc.cc` 등)은 그대로 유지되며, 
리팩토링된 버전들(`*_refactored.cc`)과 병행 사용 가능합니다.

## 문제 해결

### 컴파일 오류
- ROOT 환경이 제대로 설정되었는지 확인
- 필요한 헤더 파일들이 모두 존재하는지 확인

### 런타임 오류
- `config.txt` 파일이 실행 디렉토리에 있는지 확인
- 파일 경로가 올바른지 확인
- 매핑 파일 경로 확인

### 설정 값 관련
- 설정 파일의 문법이 올바른지 확인 (key=value, 공백 없이)
- 주석은 `#`으로 시작

### 보정 상수 관련
- 보정 상수 파일이 올바른 위치에 있는지 확인
- 파일 형식이 올바른지 확인
- 프로그램 출력에서 어떤 보정 상수가 사용되었는지 확인

### 배치 스크립트 관련
- 실행 권한이 있는지 확인: `chmod +x run_analysis_chain.sh`
- 모든 필요한 실행 파일이 컴파일되었는지 확인

## 분석 예시

### 🚀 자동 분석 예시 (권장)
```bash
# 단순한 원클릭 분석
./run_analysis_chain.sh 1234 10000 8

# 결과: 
# ✅ DWC analysis completed
# ✅ Initial calibration analysis completed  
# ✅ Calibration constants calculated
# ✅ Final analysis completed with calculated constants
# 🎉 Analysis chain completed successfully!
```

### 🔧 수동 분석 예시
```bash
# 완전한 분석 체인 실행 예시
echo "Starting TB2025 analysis for Run 1234, Module 8..."

# 1. DWC 분석
./draw_DWC_refactored 1234 10000
echo "✓ DWC analysis completed"

# 2. 초기 보정 (기본 상수 사용)
./calib_singleModule_refactored 1234 10000 8
echo "✓ Initial calibration analysis completed"

# 3. 정밀 보정 상수 계산
./calc_calib_refactored 1234 8
echo "✓ Precision calibration constants calculated"

# 4. 최종 분석 (계산된 상수 자동 적용)
./calib_singleModule_refactored 1234 10000 8
echo "✓ Final analysis completed with calculated constants"

echo "Analysis chain completed!"
```

## 향후 개선 계획

1. 다중 모듈 동시 분석 지원
2. 결과 시각화 기능 강화  
3. 웹 기반 결과 뷰어
4. 로그 시스템 구현
5. 실시간 분석 모니터링

## 문의 사항

코드 관련 문의나 개선 제안은 언제든지 환영합니다. 