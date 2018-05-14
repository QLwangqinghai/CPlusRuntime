# CPlusRuntime

## 内存布局

| 字段名 |  可选  |    字节长度    | 备注 | 
| :---: | :--: | :--------------: | :-----------: |
| customInfo | Y | 变长，取决于Type的 customInfoSize * CPContentAligentBlock|  |
| contentSize | Y | 0 或 CPContentAligentBlock|  |
| type | N | sizeof(void \*） | 必须非空 | 
| active | N | CPContentAligentBlock | 当 type.base.contentHasPadding == 1 && active.contentSize == CPMaxContentSizeInActiveInfo 时contentSize 有值; type.base.contentHasPadding == 1 && active.contentSize < CPMaxContentSizeInActiveInfo && active.contentSize > 0 时contentSize 无值， 此时contentSize = active.contentSize * CPContentAligentBlock |


## 生命周期

### autoDealloc

alloc
init
release prepareDealloc
callback->didPrepareDealloc
deallocing
callback->willDealloc
callback->deinit
dealloc




### not autoDealloc

alloc
init
release
callback->didResignActive









