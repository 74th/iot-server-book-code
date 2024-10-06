#!/bin/bash
set -xe
export AWS_ACCESS_KEY_ID=<アクセスキー>
export AWS_SECRET_ACCESS_KEY=<シークレットアクセスキー>
export AWS_REGION=ap-northeast-1
export AWS_HOSTED_ZONE_ID=<ホストゾーンID>
lego --domains <ドメイン> --email <メールアドレス> --dns route53 --accept-tos=true renew
