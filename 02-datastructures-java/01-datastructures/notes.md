# Data structures

문제 해결 과정에서 **semantic coherence(의미적 일관성)** 을 포착하는 것은 필수적이다. 각 모듈은 **semantic knot(의미적 매듭)** 로서 관련된 책임, 데이터, 그리고 연산을 하나의 의미적 맥락 속에서 통합하여 일관된 추상화 경계를 만든다. 매듭은 재귀적으로 더 작은 단위로 분해될 수 있으며, 각 수준에서 일관성을 유지한다. 여기서 “재귀적 분해”란 동일한 의미 기준과 규칙이 하위 단위에도 반복적으로 적용된다는 의미이다.

상위 모듈을 분해하면 하위 모듈들도 각각 작은 매듭으로서 명확한 인터페이스와 불변 조건(invariant)을 갖는다. 이러한 자기-유사성(self-similarity)은 각 수준에서 동일한 계약(Contract)과 추론 단위를 유지하고, 그 결과 합성 가능한 설계와 지역적 추론(compositional reasoning)이 가능하도록 한다.

**Linear data structure(선형 자료 구조)** 인 list, stack, queue는 요소를 순차적 흐름에 따라 조직하는 의미적 매듭 역할을 한다. 반면 search tree, heap, hash table, graph와 같은 **non-linear data structure(비선형 자료 구조)** 는 계층 구조, 우선순위, 키 기반 접근, 복잡한 관계를 통해 데이터를 조직하는 의미적 매듭을 만든다.

프로그래밍에서 특정 작업을 별도의 함수로 추출하는 것은 이러한 모듈화 원칙을 구현하는 행위이다. 잘 정의된 함수는 명확한 인터페이스, 단일 책임(single responsibility), 그리고 독립적으로 추론 가능한 불변 조건을 지닌 강력한 의미 단위(semantic unit)를 형성한다.
