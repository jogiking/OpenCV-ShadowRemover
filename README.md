# OpenCV-ShadowRemover

## 결과물 (Before - After)

<div>
<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844367-5f536680-c8f7-11ea-906c-efd4ad8872ad.jpg">
<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844406-ba855900-c8f7-11ea-8ff0-475c05810796.jpg">

<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844369-5febfd00-c8f7-11ea-90c9-da6fb2a3dfee.jpg">
<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844408-bbb68600-c8f7-11ea-9a56-ab769ff477e3.jpg">

<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844370-60849380-c8f7-11ea-81d3-694d4e2b9e7d.jpg">
<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844409-bbb68600-c8f7-11ea-9db9-d339ca075beb.jpg">

<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844371-611d2a00-c8f7-11ea-819f-04973bf98f60.jpg">
<img width = "500" src = "https://user-images.githubusercontent.com/38206212/87844410-bc4f1c80-c8f7-11ea-871e-641af01b4eb9.jpg">
</div>

## 구현 아이디어의 과정

가장 먼저 생각했던 프로그램의 흐름은 다음과 같다.

> 1 . 이미지가 뿌옇게 찍혀 나와 있을 수 있으므로 이미지 윤곽을 분명하게 만들어준다.

> 2 . 선명하게 만든 이미지에서 각 점을 불러와서 찍을지 안 찍을지를 결정하는 과정을 거쳐서 변환된 이미지를 완성한다.

첫 번째 과정에서 Un-sharp Masking을 사용하는 방법을 사용했었다. 그렇지만 이 문제 상황에 더 적절한 커널을 사용하고 싶었다.
글자 영역의 검출을 한다면 좀더 선명한 이미지를 얻을 수 있을 것이라고 생각했다.
그래서 blur를 이용한 방법을 사용하지 않고 Cross-correlation filtering(상호-상관 필터)를 사용했다.
커널값을 구상하기 위해서 Laplacian mask(라플라시안 마스크)를 이용하기로 했다.

두 번째 과정은 이 점을 찍을지 안 찍을지에 대한 판단에 대한 것이다. 글자는 분명히 주변 배경보다 어둡다.
그리고 현재 주어진 상황에서는 주변 배경은 종이이다. 종이는 글자를 보여주기 때문에 이미지에서 가장 밝은 영역일 것이다.
주변 픽셀들의 평균값이나 중간값을 구한 뒤, 이 값이 종이- 현재 가장 밝은 값이라고 설정을 한다. 이렇게 한다면 그림자가 있어도 그림자영역의 평균이 가장 밝은 부분으로 인식될 것이다.

만약 여기에 지금 가져온 픽셀이 이 평균(종이)보다 어둡다면 점을 찍으면 된다.
여기서 pallete가 (0,0,0), (주변배경의 평균값)의 색간거리에 대한 Image Threshholding을 사용하기로 했다.

추가적으로 주변 배경의 평균값을 계산할 때, 고속으로 처리하기 위하여 Fastest Mean Filter에서 만들었던 SAT(Summed Area Table)를 사용했다.


##  구현

<img src = "https://user-images.githubusercontent.com/38206212/87844547-f79e1b00-c8f8-11ea-9d45-1c33ce5742c4.JPG">

처음에는 기본 Laplacian mask를 (-1, -1, -1, -1, 9, -1, -1, -1, -1)가 결과는 글자 자체는 상당히 선명하지만
글자 주변부에서 검은 점들이 지저분하게 찍혀있어  (0, -1, 0, -1, 5, -1, 0, -1, 0) 로 수정했다.
위 사진을 보면 새로 만든 마스크에서는 글자 주변이 깨끗하다는 것을 확인 할 수 있다.
하지만 이때 기본 라플라시안 마스크를 사용했을 때 발생하지 않았던 문제가 발생했다.
글자가 끊긴 부분이 발생했다. 이때 Image Threshholding에서 추가한 부분이 있다.
우선 SAT(Summed Area Table)를 이용하여 어느 한 점의 주변 영역의 평균을 구한다.

``` c
1: for (int i = 0; i < numColor; i++)
2: {
3:      float d <- diff(f, palette[i]);
4:      if (d < min_d) then
5:      {
6:          min_color <- i;
7:          min_d <- d;
8:      }
9: }
10: if (min_color == 1) then
11: palette[1] <- cvScalar(255, 255, 255);
```

그리고 그 평균 색을 RGB(255,255,255)대신으로 palette[1]과 가장 밝은 점의 범위로 두고 거리 비교를 진행한다. 
반복문에서 가장 먼저 색이 들어가는건 palette[0]인 검은색으로 초기화 될 것이다. 
그리고 가장 밝은 색이 들어가는palette[1]에는 SAT에서 구한 평균이 들어가게 되는데,
만약에 반복문에서 가장 밝은 색에 가깝다는 판정이 나온다면 이에 따른 처리(10,11번째 줄)를 해준다. 
만약에 판정 결과 평균색과 가깝다는 결과가 나온다면 점은 (255,255,255)의 값이 들어가게 되어 흰 점이 찍히게 될 것이다.
그림자 영역이 있다고 해도 글자는 주변보다 어두울 것이므로 마찬가지로 실행된다. 결과적으로 그림자를 지운 그림이 완성이 된다.
