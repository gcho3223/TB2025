# TB2025 EM Resolution 분석 코드

이 코드는 TB2024에서 사용하던 EM Resolution 분석 코드를 TB2025 데이터에 맞게 업데이트한 버전입니다.

## 주요 변경사항 (TB2024 → TB2025)

- **Run 번호**: 10000번대 → 12000번대
- **EM 에너지 범위**: 10-100 GeV → 10-120 GeV  
- **중심 타워**: M8-T2 → M5-T2 (기본값), M4-T2도 선택 가능
- **네임스페이스**: TB2024 → TB2025

## 사용 가능한 데이터 (TB2025)

### EM 빔 데이터 ✅
- **에너지 범위**: 10, 20, 30, 40, 60, 80, 100, 120 GeV
- **중심 타워**: M5-T2 (기본값) 또는 M4-T2
- **데이터 위치**: `../../Hist/EmScanRun/Hist_Run_{run_number}.root`

### 하드론 빔 데이터 ❌
- Muon, Pion, Kaon, Proton 빔 데이터는 아직 TB2025에서 사용 불가

## 사용법

### 1. 기본 사용법 (M5-T2 중심)
```bash
root -l -b -q produce_emResol.cc'("em")'
```

### 2. M4-T2를 중심으로 사용
```bash
root -l -b -q produce_emResol.cc'("em", "m4")'
```

### 3. NoNoise 옵션 (노이즈 텀 제외, 110 GeV까지 피팅)
```bash
root -l -b -q produce_emResol.cc'("em", "nn")'
root -l -b -q produce_emResol.cc'("em", "nnm4")'  # M4-T2 + NoNoise
```

### 4. Reverse 옵션 (1/√E vs σ/E 그래프, TB2022 스타일)
```bash
root -l -b -q produce_emResol.cc'("em", "r")'
root -l -b -q produce_emResol.cc'("em", "rm4")'   # M4-T2 + Reverse
```

### 5. NoNoise + Reverse 조합
```bash
root -l -b -q produce_emResol.cc'("em", "nnr")'
root -l -b -q produce_emResol.cc'("em", "nnrm4")' # M4-T2 + NoNoise + Reverse
```

## 옵션 설명

| 옵션 | 설명 |
|------|------|
| `nn` | NoNoise 모드: 노이즈 텀을 제외하고 피팅 (30-110 GeV 범위) |
| `r`  | Reverse 모드: 1/√E vs σ/E 그래프 생성 (TB2022 스타일) |
| `m4` | M4-T2를 중심 타워로 사용 (기본값은 M5-T2) |

## 출력 파일

결과는 `./plot/em/` 디렉토리에 저장됩니다:
- 해상도 vs 에너지 플롯
- 선형성 vs 에너지 플롯 (NoNoise 옵션이 아닌 경우)
- 에너지별 개별 분포 플롯

## 파일 구조

```
analysis/Guk/plots/
├── EMResolution.h          # 메인 분석 클래스
├── produce_emResol.cc      # 실행 스크립트
├── header/
│   ├── SimplePlotTools.h   # 플롯 도구
│   ├── RunInfo.h          # TB2024/TB2025 런 정보
│   └── GausFitter.h       # 가우시안 피팅 도구
└── plot/                  # 결과 플롯 저장 위치
```

## 문제 해결

### 1. 데이터 파일을 찾을 수 없는 경우
- `../../Hist/EmScanRun/` 경로에 히스토그램 파일이 있는지 확인
- 파일명이 `Hist_Run_{run_number}.root` 형식인지 확인

### 2. 하드론 빔 데이터를 사용하려고 하는 경우
```
[produce_emResol] WARNING: Muon beam data is not available for TB2025 yet.
```
→ 현재 TB2025에서는 EM 빔 데이터만 사용 가능합니다.

### 3. 중심 타워 데이터가 없는 경우
- M5-T2와 M4-T2 데이터가 모두 있는지 확인
- RunInfo.h의 `TB2025::map_emScanRun`에서 해당 타워의 런 번호 확인

## 개발자 정보

TB2024 코드를 TB2025에 맞게 포팅 완료
- 기존 TB2024 코드의 모든 기능 유지
- TB2025 데이터 구조에 맞게 업데이트
- 새로운 에너지 범위 (120 GeV까지) 지원 