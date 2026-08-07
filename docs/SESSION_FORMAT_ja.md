# Session Manager の XML 形式

> この文書では、Session Manager がセッション情報を保存する XML 形式について日本語で説明します。
> 仕様上の正本は `SESSION_FORMAT.md` です。

## 保存ファイル

Session Manager のセッション情報は、次のファイルに保存されます。

```text
saved-sessions.xml
```

## XML の基本構造

Session Manager が書き出すファイルは、XML 宣言と `saved-sessions` ルート要素で構成されます。

もっとも単純な形は次のようになります。

```xml
<?xml version="1.0" encoding="UTF-8"?>
<saved-sessions>
</saved-sessions>
```

## session 要素

1つの保存済みセッションを、1つの `session` 要素で表します。

たとえば、`work` という名前のセッションは次のようになります。

```xml
<session name="work">
</session>
```

セッション名は `name` 属性に保存します。

`name` 属性は必須で、空にはできません。

通常の Session Manager の操作では、同じ名前のセッションが複数作られないようになっています。ただし、XML ファイルを手動で編集して重複するセッション名を記述した場合、現在のパーサーはそれをエラーにはしません。

また、通常の操作でセッション名を比較するときは、ASCII の英字について大文字と小文字を区別しません。

そのため、

- `Work`
- `work`
- `WORK`

は同じセッション名として扱われます。

セッション名に XML で特別な意味を持つ文字が含まれている場合は、ファイルへの書き込み時に適切にエスケープします。

## file 要素

セッションに含まれる1つの文書を、1つの `file` 要素で表します。

例：

```xml
<file path="file:///home/user/Documents/example.txt"/>
```

文書の場所は `path` 属性に保存します。

`path` 属性は必須で、空にはできません。

属性名は `path` ですが、実際に保存される値は通常のファイルシステム上のパスではなく、GIO URI です。

また、`file` 要素は必ず `session` 要素の中に置きます。

ファイル URI に XML で特別な意味を持つ文字が含まれている場合は、書き込み時に適切にエスケープします。

複数の `file` 要素がある場合、その並び順が復元時のタブの順序として使われます。

## Previous Session

`Previous Session` は、Session Manager が自動復元に使用する特別なセッション名です。

ここには、最後に記録された、開いている保存済み文書の一覧が格納されます。

`Previous Session` という名前は予約されているため、ユーザーが同じ名前のセッションを作成したり、既存のセッションをこの名前に変更したりすることはできません。

また、`Previous Session` 自体をユーザーが削除したり、名前を変更したりすることもできません。

## 保存ファイルの例

`Previous Session` と `work` という2つのセッションが保存されている場合、XML は次のようになります。

```xml
<?xml version="1.0" encoding="UTF-8"?>
<saved-sessions>
  <session name="Previous Session">
    <file path="file:///home/user/Documents/example1.txt"/>
    <file path="file:///home/user/Documents/example2.txt"/>
  </session>
  <session name="work">
    <file path="file:///home/user/Documents/work.txt"/>
  </session>
</saved-sessions>
```

## 読み込み時のチェック

現在のパーサーが認識する要素は、次の3種類です。

- `saved-sessions`
- `session`
- `file`

これ以外の未知の XML 要素がある場合は、読み込みエラーになります。

一方、未知の属性が追加されていても、その属性は無視します。

また、次の場合も読み込みに失敗します。

- `session` 要素に有効な `name` 属性がない
- `file` 要素が `session` 要素の外にある
- `file` 要素に有効な `path` 属性がない
- XML が整形式になっていない

ただし、現在のパーサーは XML の構造を完全に検証しているわけではありません。

たとえば、`saved-sessions` ルート要素が厳密に1つだけ存在するかどうかは明示的に確認していません。また、手動で編集された XML に同じ名前のセッションが複数あっても、読み込み時にはエラーにしません。

## 従来形式との互換性

この XML 形式は、従来の gedit Session Saver で使われていた構造を引き継いでいます。

基本となる要素と属性は次のとおりです。

- `saved-sessions`
- `session`
- `name`
- `file`
- `path`

現在は、XML 形式そのもののバージョンを示す要素や属性は追加していません。
